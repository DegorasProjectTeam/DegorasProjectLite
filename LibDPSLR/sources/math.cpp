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

#include <numeric>

// ========== C++ INCLUDES =============================================================================================
#include <array>
#include <algorithm>
#include <limits>
#include <math.h>
// =====================================================================================================================


// ========== PROJECT INCLUDES =========================================================================================
#include "includes/dpslr_math.h"
// =====================================================================================================================

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr
{
namespace math
{
// =====================================================================================================================

euclid_lldiv_t euclidDivLL(long long a, long long b)
{
    long long r = a % b;
    r =  r >= 0 ? r : r + std::abs(b);
    long long q = (a - r) / b;
    return {q, static_cast<unsigned long long>(r)};
}

}
} // END NAMESPACES
// =====================================================================================================================



