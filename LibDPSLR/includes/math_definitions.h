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

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr
{
namespace math
{
// ========== CONSTANTS ===============================================================================================
constexpr long double pi = 3.14159265358979323846264338327950288419716939937510L;   // Pi number.
constexpr double c = 299792458.0;                                                   // Speed of light (m/s).
constexpr double kSecondToPicosecond = 1e12;
constexpr double kNanometerToMicrometer = 0.001;
// ====================================================================================================================

// Histcount result for each bin: {count, left limit, right limit}
template <typename C>
using HistcountBinType = std::tuple<unsigned, typename C::value_type, typename C::value_type>;

// Histcount result: a vector with HistcountBinType tuples for each bin
template <typename C>
using HistcountRetType = std::vector<HistcountBinType<C>>;

// ========== ENUMS ====================================================================================================
// Enum for represent the possible errors from the lagrange polynomial interpolator.
enum class LagrangeResult
{
    NOT_ERROR,
    NOT_IN_THE_MIDDLE,
    X_OUT_OF_BOUNDS,
    DATA_SIZE_MISMATCH
};

enum class PolyFitRobustMethod
{
    NO_ROBUST,
    BISQUARE_WEIGHTS
};

// =====================================================================================================================

// ========== STRUCTS =================================================================================================
// Strcut for storing the euclidean division result,
struct euclid_lldiv_t
{
    long long q;
    unsigned long long r;
};
// ====================================================================================================================

}
}
