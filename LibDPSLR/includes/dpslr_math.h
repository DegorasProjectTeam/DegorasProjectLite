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

// ========== INTERNAL INCLUDES ========================================================================================
#include "math.tpp"
#include "math_definitions.h"
#include "libdpslr_global.h"
#include "class_matrix.h"
// =====================================================================================================================


// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr
{
namespace math
{
// =====================================================================================================================


// ========== FUNCTIONS ===============================================================================================

template <typename T>
double rad(T degree)
{
    return (degree*(math::pi/180.0));
}

template <typename T>
double deg(T radian)
{
    return (radian*(180.0/math::pi));
}

template <typename T>
T normalizeAngle(T x, T x_min, T x_max)
{
    return std::fmod(((std::fmod((x - x_min),(x_max - x_min)))+(x_max - x_min)), (x_max - x_min)) + x_min;
}

// Euclidean division for long long data.
LIBDPSLR_EXPORT euclid_lldiv_t euclidDivLL(long long a, long long b);

// Floating point comparation. 1:a>b, 0:a=b, -1:a<b. Default epsilon is epsilon of type
template <typename T>
std::enable_if_t<std::is_floating_point<T>::value, int>
compareFloating(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return dpslr::math_private::compareFloating(a, b, epsilon);
}

// Euclidean 3D rotation matrix.
template <typename T>
void euclid3DRotMat(int axis, T angle, Matrix<T> &matrix)
{
    return dpslr::math_private::euclid3DRotMat(axis, angle, matrix);
}

// Lagrange polinomial interpolation.
template <typename T, typename U>
LagrangeResult lagrangeInterp(const std::vector<T>& x, const Matrix<T>& Y, unsigned int degree,
                              T x_interp, std::vector<U>& y_interp)
{
    return dpslr::math_private::lagrangeInterp(x, Y, degree, x_interp, y_interp);
}

// Mean statistic function.
template <typename T>
T mean(const std::vector<T>& x)
{
    return dpslr::math_private::mean(x);
}

/**
 * @brief Median of distribution in x.
 * @param x, a vector containing the distribution data.
 * @return The median calculation for data in x.
 */
template <typename T>
T median(const std::vector<T>& x)
{
    return dpslr::math_private::median(x);
}

// Skew statistic function.
template <typename T>
T skew(const std::vector<T>& x)
{
    return dpslr::math_private::skew(x);
}

// Kurtosis statistic function with Pearson's definition: Normal distribution kurtosis is 3.
template <typename T>
T kurtosis(const std::vector<T> &x)
{
    return dpslr::math_private::kurtosis(x);
}

/**
 * @brief Variance of distribution in x.
 * @param x, a vector containing the distribution data.
 * @return The variance of the distribution in x.
 */
template <typename T>
T var(const std::vector<T>& x)
{
    return dpslr::math_private::var(x);
}

/**
* @brief Standard deviation of distribution in x.
* @param x, a vector containing the distribution data.
* @return The standard deviation of the distribution in x.
*/
template <typename T>
T stddev(const std::vector<T>& x)
{
    return dpslr::math_private::stddev(x);
}

/**
 * @brief Root Mean Squared of distribution in x.
 * @param x, a vector containing the distribution data.
 * @return The Root Mean Squared calculation for data in x.
 */
template <typename T>
T rms(const std::vector<T>& x)
{
    return dpslr::math_private::rms(x);
}

/**
 * @brief Leverage of distribution in x.
 * @param x, a vector containing the distribution data.
 * @return The leverage calculation for data in x.
 */
template <typename T>
T leverage(const std::vector<T>& x)
{
    return dpslr::math_private::leverage(x);
}

/**
 * @brief Gets the polynomial fit coefficients for x,y.
 * @param x, a vector with the independent variable values.
 * @param y, a vector with the dependent variable values. It must have the same size as x.
 * @param degree, the degree of the polynomial fit
 * @param w, a vector with the weights applied to each observation. It must be empty or have the same size as x.
 * @param robust, the robust fit method selected
 * @return The coefficients of the polynomial fit for x and x, or empty vector if x and x sizes are not equal.
 *         The order of the coefficients in the returned vector is c[0] + c[1] * x + c[2] * x^2 + ... + c[n] * x^n.
 */
template <typename T, typename Ret = T>
std::vector<Ret> polynomialFit(const std::vector<T>& x, const std::vector<T>& y, unsigned int degree,
                               const std::vector<T>& w = std::vector<T>(),
                               dpslr::math::PolyFitRobustMethod robust = dpslr::math::PolyFitRobustMethod::NO_ROBUST)
{
    return dpslr::math_private::polynomialFit(x, y, degree, w, robust);
}

/**
 * @brief Resolve for @param x the polynomial given by its coefficients in @param coefs.
 * @param coefs, the coefficients of the polynomial: coefs[0] + coefs[1] * x + cofes[2] * x^2 + ... + coefs[n] * x^n
 * @param x, the independent variable for resolving the polynomial.
 * @return The solution of the polynomial: coefs[0] + coefs[1] * x + cofes[2] * x^2 + ... + coefs[n] * x^n.
 *         If @param coefs is empty, returns 0.
 */
template <typename T>
T applyPolynomial(const std::vector<T>& coefs, T x)
{
    return dpslr::math_private::applyPolynomial(coefs, x);
}


/**
 * @brief Removes polynomial trend from the data in y.
 * @param x, a vector with the independent variable values.
 * @param y, a vector with the dependent variable values. It must have the same size as x.
 * @param degree, the degree of the polynomial fit using for detrend.
 * @return The data with the trend removed.
 * @note When degree = 1, detrend removes the linear trend.
 */
template <typename T, typename Ret = T>
std::vector<Ret> detrend(const std::vector<T>& x, const std::vector<T>& y, unsigned int degree)
{
    return dpslr::math_private::detrend(x, y, degree);
}

/**
 * @brief Removes polynomial trend from the data in y, using xinterp, yinterp to form the polynomial fit
 * @param x, a vector with the independent variable values.
 * @param y, a vector with the dependent variable values. It must have the same size as x.
 * @param xinterp, a vector with the independent variable for polynomial fit.
 * @param yinterp, a vector with the dependent variable for polynomial fit. It must have the same size as xinterp.
 * @param degree, the degree of the polynomial fit using for detrend.
 * @return The data with the trend removed.
 * @note When degree = 1, detrend removes the linear trend.
 */
template <typename T, typename Ret = T>
std::vector<Ret> detrend(const std::vector<T>& x, const std::vector<T>& y,
                         const std::vector<T>& xinterp, const std::vector<T>& yinterp, unsigned int degree)
{
    return dpslr::math_private::detrend(x, y, xinterp, yinterp, degree);
}

template <typename Container>
HistcountRetType<Container> histcounts1D(const Container& data)
{
    // TODO. AUTOMATIC BINDING ALGORITHM (SCOTT).
}

template <typename Container>
HistcountRetType<Container> histcounts1D(const Container& data, size_t nbins,
                                         typename Container::value_type min_edge,
                                         typename Container::value_type max_edge)
{
    return dpslr::math_private::histcounts1D(data, nbins, min_edge, max_edge);
}

template <typename Container>
HistcountRetType<Container> histcounts1D(const Container& data, size_t nbins)
{
    return dpslr::math_private::histcounts1D(data, nbins);
}

} // END NAMESPACE MATH
// =====================================================================================================================

} // END NAMESPACE DPSLR
// =====================================================================================================================

// =====================================================================================================================
