/***********************************************************************************************************************
 * Copyright 2023 Degoras Project Team
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they will be approved by the
 * European Commission - subsequent versions of the EUPL (the "Licence");
 *
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the Licence is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the Licence for the
 * specific language governing permissions and limitations under the Licence.
 **********************************************************************************************************************/

#pragma once

#include <vector>
#include <numeric>
#include <cmath>
#include <functional>
#include <algorithm>
#include <omp.h>

#include "helpers.h"
#include "math_definitions.h"
#include "math_operators.tpp"
#include "class_matrix.h"

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr {
namespace math_private  {
// =====================================================================================================================

template <typename T>
std::enable_if_t<std::is_floating_point<T>::value, int>
compareFloating(T a, T b, T epsilon)
{
    T aux = a - b;
    return std::abs(aux) < epsilon ? 0 : std::signbit(aux) ? -1 : 1;
}

template <typename T>
T mean(const std::vector<T> &X)
{
    return std::accumulate(X.begin(), X.end(), 0.0) / X.size();
}

template <typename T>
T skew(const std::vector<T> &X)
{
    T mean = mean(X);
    T u2 = 0.0;
    T u3 = 0.0;
    for (const auto& x : X)
    {
        T x_minus_mean = x - mean;
        u2 += std::pow(x_minus_mean, 2);
        u3 += std::pow(x_minus_mean, 3);
    }
    u2 /= X.size();
    u3 /= X.size();

    return u3 / std::pow(u2, 1.5);
}

template <typename T>
T kurtosis(const std::vector<T> &X)
{
    T mean = mean(X);
    T u2 = 0.0;
    T u4 = 0.0;
    for (const auto& x : X)
    {
        T x_minus_mean = x - mean;
        u2 += std::pow(x_minus_mean, 2);
        u4 += std::pow(x_minus_mean, 4);
    }
    u2 /= X.size();
    u4 /= X.size();

    return u4 / std::pow(u2, 2);
}

template <typename T>
T var(const std::vector<T>& X)
{
    T m = mean(X);
    T u2 = 0.0;

    for (const auto& x : X)
    {
        T x_minus_mean = x - m;
        u2 += std::pow(x_minus_mean, 2);
    }

    return u2 / X.size();
}

template <typename T>
T stddev(const std::vector<T>& X)
{
   return std::sqrt(var(X));
}

template <typename T>
T rms(const std::vector<T>& X)
{
    return std::sqrt(std::inner_product(X.begin(), X.end(), X.begin(), T()) / X.size());
}

template <typename T>
T median(const std::vector<T>& x, bool sorted = false)
{
    if (x.empty())
        return 0;

    T med;

    // If x is sorted, avoid copy and sort. Once data is sort, choose median element:
    // centre if even elements, mean of two centre elements othersie)
    if (sorted)
    {
        med = (0 == x.size() % 2) ? (x[x.size() / 2] + x[(x.size() / 2) - 1]) / 2. : x[x.size() / 2];
    }
    else
    {
        auto x_sort = x;
        std::sort(x_sort.begin(), x_sort.end());
        med = (0 == x.size() % 2) ? (x_sort[x.size() / 2] + x_sort[(x.size() / 2) - 1]) / 2. : x_sort[x.size() / 2];
    }

    return med;
}

// TODO: change this and fit methods to allow matrix?
template <typename T, typename Ret = T>
std::vector<Ret> leverage(const std::vector<T>& x)
{
    // Leverage formula: hi = 1/n + ((xi - xm)^2 / sumsqr(x - xm))
    auto m = mean(x);
    std::vector<T> leverages;
    // First insert x minus mean
    std::transform(x.begin(), x.end(), std::back_inserter(leverages), [m](const auto& e){return e - m;});
    // Calculate deviations sum sqr
    auto devsqr = std::inner_product(leverages.begin(), leverages.end(), leverages.begin(), T());
    auto n_inv = 1./ x.size();
    std::transform(leverages.begin(), leverages.end(), leverages.begin(),
                   [devsqr, n_inv](const auto& e){return n_inv + (e*e / devsqr);});
    return leverages;

}

template <typename T, typename Ret = T>
std::vector<Ret> computeBisquareWeights(const std::vector<T>& x, const std::vector<T>& y, const std::vector<T>& yc,
                                        const double K = 4.685)
{
    using namespace math_operators_private;

    auto leverages = leverage(x);
    auto resids = y - yc;
    decltype(resids) resids_abs;
    std::transform(resids.begin(), resids.end(), std::back_inserter(resids_abs), [](const auto& v){return std::abs(v);});
    // Median absoulte deviation
    auto mad = median(resids_abs);
    double s = mad/0.6745;
    double Ks = K*s;
    std::vector<T> u;
    std::vector<Ret> w;

    // Adjust residuals using radj = r / sqrt(1 - h)
    std::transform(resids.begin(), resids.end(), leverages.begin(), resids.begin(),
                   [](const auto& r, const auto& h){return r/std::sqrt(1 - h);});

    // Standarize adjusted residuals using u = radj / K*s
    std::transform(resids.begin(), resids.end(), std::back_inserter(u), [Ks](const auto& r){return r / Ks;});

    // Compute robust weights using u = (1 - u^2)^2 if abs(u) < 1 or 0 otherwise
    std::transform(u.begin(), u.end(), std::back_inserter(w),
                   [](const auto& e){return std::abs(e) < 1 ? std::pow(1 - std::pow(e,2), 2) : 0;});

    return w;
}

template <typename T>
T applyPolynomial(const std::vector<T>& coefs, T x)
{
    if (coefs.empty())
        return T(0);

    std::vector<T> x_pow;

    for (std::size_t i = 0; i < coefs.size(); i++)
        x_pow.push_back(std::pow(x, i));

    return std::inner_product(coefs.begin(), coefs.end(), x_pow.begin(), T(0));
}

template <typename T, typename Ret = T>
std::vector<Ret> polynomialFit(const std::vector<T>& x, const std::vector<T>& y, unsigned int degree,
                               const std::vector<T>& w = std::vector<T>(),
                               dpslr::math::PolyFitRobustMethod robust = dpslr::math::PolyFitRobustMethod::NO_ROBUST)
{

    // Variable declaration
    std::vector<Ret> coefs(degree+1);     // Resulting coeficients.
    std::vector<Ret> sigma_x(2*degree+1); // Values of sigma(xi),sigma(xi^2),sigma(xi^2n).
    dpslr::math::Matrix<Ret> normal_matrix(degree+1, degree+2);
    unsigned int i, j, k;

    // Avoid X and Y size mismatch as well as weights mismatch if it is not empty
    if (x.size() != y.size() || (!w.empty() && x.size() != w.size()))
        return {};

    for (i = 0; i < 2*degree+1; i++)
    {
        sigma_x[i] = 0;
        for(j = 0; j < x.size(); j++)
        {
            auto sum = pow(x[j],i);
            if (!w.empty())
                sum *= w[j];
            sigma_x[i] += sum;
        }
    }

    // Build the Normal matrix by storing the corresponding coefficients (except for the last column of the matrix).
    for (i = 0; i <= degree; i++)
        for (j = 0; j <= degree; j++)
            normal_matrix[i][j] = sigma_x[i+j];

    // Compute last column of Normal matrix
    // Values of sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
    for (i = 0; i < degree+1; i++)
    {
        normal_matrix[i][degree+1] = 0;
        for (j = 0; j < x.size(); j++)
        {
            auto sum = pow(x[j],i) * y[j];
            if (!w.empty())
                sum *= w[j];

            normal_matrix[i][degree+1] += sum;
        }
    }

    // Increment the degree.
    degree++;

    // TODO: this may be optimized reducing number of swaps. It also does not control if matrix is singular.
    // Maybe gaussian elimination could be a separate function
    // From now, gaussian elimination starts solving the set of linear equations (pivotisation)
    for (i = 0; i < degree; i++)
        for (k = i + 1; k < degree; k++)
            if (normal_matrix[i][i] < normal_matrix[k][i])
                normal_matrix.swapRows(i,j);

    // Loop to perform the gaussian elimination
    for (i = 0; i < degree - 1; i++)
        for (k = i + 1; k < degree; k++)
        {
            Ret t = normal_matrix[k][i]/normal_matrix[i][i];
            for (j = 0; j <= degree; j++)
                normal_matrix[k][j] = normal_matrix[k][j] - t*normal_matrix[i][j];
        }

    for (int i = static_cast<int>(degree) - 1; i >= 0; i--)
    {
        coefs[i] = normal_matrix[i][degree];

        for (int j = 0; j < static_cast<int>(degree); j++)
            if (j != i)
                coefs[i] -= normal_matrix[i][j]*coefs[j];

        coefs[i] /= normal_matrix[i][i];
    }

    // If robust method is selected, calculate weights and recalculate coefficients
    if (dpslr::math::PolyFitRobustMethod::BISQUARE_WEIGHTS == robust)
    {
        // In bisquare weights, weights are recalculated iteratively until coefficients converge
        int i = 0;
        const int limit = 400;
        const double threshold = 1e-6;
        std::vector<Ret> prev_coefs(degree+1);
        std::vector<T> y_c;
        std::vector<T> calc_weights;
        bool converged;

        do
        {
            // Compute calculated y for current fit
            std::transform(x.begin(), x.end(), std::back_inserter(y_c),
                           [&coefs](const auto& xi){return applyPolynomial(coefs, xi);});

            // Compute bisquare weights
            calc_weights = computeBisquareWeights(x, y,  y_c);

            // Redo fit with new weights and check if they converged
            prev_coefs = std::move(coefs);
            coefs = polynomialFit(x, y, degree - 1, calc_weights);

            int j = 0;
            converged = true;
            while(j < coefs.size() && converged )
            {
                converged &= std::abs(coefs[j] - prev_coefs[j]) < threshold;
                j++;
            }

            i++;
        } while (i < limit && !converged);
    }

    // Return the coefs
    return coefs;
}

template <typename T, typename Ret = T>
std::vector<Ret> detrend(const std::vector<T>& x, const std::vector<T>& y, unsigned int degree)
{
    // Return vector.
    std::vector<Ret> ret;

    // Calculate the polynomial fit.
    auto coefs = polynomialFit(x, y, degree);

    // Apply the polynomial fit.
    for (std::size_t i = 0; i < x.size(); i++)
    {
        auto result = applyPolynomial(coefs, x[i]);
        ret.push_back(y[i] - result);
    }

    // Return the data with the fit removed.
    return ret;
}

template <typename T, typename Ret = T>
std::vector<Ret> detrend(const std::vector<T>& x, const std::vector<T>& y,
                         const std::vector<T>& xinterp, const std::vector<T>& yinterp, unsigned int degree)
{
    // Return vector.
    std::vector<Ret> ret;

    // Calculate the polynomial fit.
    auto coefs = polynomialFit(xinterp, yinterp, degree);

    // Apply the polynomial fit.
    for (std::size_t i = 0; i < x.size(); i++)
    {
        auto result = applyPolynomial(coefs, x[i]);
        ret.push_back(y[i] - result);
    }

    // Return the data with the fit removed.
    return ret;
}

template <typename C>
dpslr::math::HistcountRetType<C> histcounts1D(const C& data, size_t nbins,
                                              typename C::value_type min_edge, typename C::value_type max_edge)
{
    // Convenient alias.
    using ConType = typename C::value_type;

    // Return container.
    std::vector<std::tuple<unsigned, ConType, ConType>> result(nbins);

    // Get the division.
    ConType div = (max_edge - min_edge) / nbins;

    // Parallel loop for each bin.
    omp_set_num_threads(omp_get_num_procs());
    #pragma omp parallel for
    for (size_t i = 0; i < nbins; i++ )
    {
        // Update the next counter.
        ConType min = min_edge + i * div;
        ConType max = min + div;
        // Count the data in the bin.
        unsigned counter = helpers::countBin(data, min,  max);
        // Push the new data in the result vector, and update the min counter.
        result[i] = {counter, min, max};
    }

    // Return the result.
    return result;
}

template <typename C>
dpslr::math::HistcountRetType<C> histcounts1D(const C& data, size_t nbins)
{
    // Convenient alias.
    using ConType = typename C::value_type;

    // Return container.
    std::vector<std::tuple<unsigned, ConType, ConType>> result(nbins);

    // Get the minimum and maximum values.
    auto minmax = std::minmax_element(data.begin(), data.end());
    ConType min_counter = *(minmax.first);
    ConType max_counter = *(minmax.second);

    // Get the division.
    ConType div = (std::abs(max_counter) + std::abs(min_counter)) / nbins;

    // Parallel loop for each bin.
    omp_set_num_threads(omp_get_num_procs());
    #pragma omp parallel for
    for (size_t i = 0; i < nbins; i++ )
    {
        // Update the next counter.
        ConType min = min_counter + i * div;
        ConType max = min + div;
        // Count the data in the bin.
        unsigned counter = helpers::countBin(data, min,  max);
        // Push the new data in the result vector, and update the min counter.
        result[i] = {counter, min, max};
    }

    // Return the result.
    return result;
}

template <typename T>
void euclid3DRotMat(int axis, T angle, dpslr::math::Matrix<T> &matrix)
{
    double s, c;
    unsigned int caxis = static_cast<unsigned int>(axis - 1);
    matrix.fill(3,3,0);
    s= std::sin(angle);
    c= std::cos(angle);
    matrix[0][0]=c;
    matrix[1][1]=c;
    matrix[2][2]=c;
    matrix[0][1]=-s;
    matrix[1][2]=-s;
    matrix[2][0]=-s;
    matrix[1][0]=s;
    matrix[2][1]=s;
    matrix[0][2]=s;
    for (unsigned i=0; i<3; i++)
    {
      matrix[i][caxis] = 0.0;
      matrix[caxis][i] = 0.0;
    }
    matrix[caxis][caxis]= 1.0;
}

template <typename T, typename U>
dpslr::math::LagrangeResult lagrangeInterp(const std::vector<T>& x, const dpslr::math::Matrix<T>& Y,
                                            unsigned int degree, T x_interp, std::vector<U>& y_interp)
{
    // 'y' must be sorted
    // Degree = number of points - 1 (order 9 -> 10 points)

    // Variables.
    unsigned int first_point;
    int aux;
    dpslr::math::LagrangeResult error = dpslr::math::LagrangeResult::NOT_ERROR;

    if (x.size() > 0 && x.size() == Y.rowSize())
    {
        // x_interp is not within x range, so it cannot be interpolated.
        if (x_interp < x[0] || x_interp > x.back())
        {
            return dpslr::math::LagrangeResult::X_OUT_OF_BOUNDS;
        }

        // Look for given value immediately after interpolation argument
        aux = 0;
        do
        {
            aux++;
        }while(x[static_cast<std::size_t>(aux)] < x_interp);

        // Get first interpolator point. The first point should leave the interpolated point in the middle.
        aux -= (degree + 1)/2;
        if (aux < 0)
        {
            first_point = 0;
            error = dpslr::math::LagrangeResult::NOT_IN_THE_MIDDLE;
        }
        else if (static_cast<unsigned int>(aux) + degree >= x.size())
        {
            first_point = static_cast<unsigned int>(x.size() - degree - 1);
            error = dpslr::math::LagrangeResult::NOT_IN_THE_MIDDLE;
        }
        else
        {
            first_point = static_cast<unsigned int>(aux);
        }

        y_interp.clear();
        y_interp.insert(y_interp.begin(), Y.columnsSize(), 0);

        // Apply Lagrange polynomial interpolation to all variables in Y.
        for (unsigned int i = first_point; i <= first_point + degree; i++)
        {
            double pj=1.0;
            for(unsigned int j = first_point; j <= first_point + degree; j++)
            {
                if (j != i) pj*=(x_interp-x[j])/(x[i]-x[j]);
            }
            for (unsigned int variable = 0; variable < Y.columnsSize(); variable++)
            {
                y_interp[variable]+=Y[i][variable]*pj;
            }
        }
    }
    else
    {
        error = dpslr::math::LagrangeResult::DATA_SIZE_MISMATCH;
    }
    return error;
}

} // END NAMESPACE MATH
// =====================================================================================================================



} // END NAMESPACE DPSLR
// =====================================================================================================================
