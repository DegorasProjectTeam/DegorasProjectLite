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

/** ********************************************************************************************************************
 * @file astronomy.cpp
 *
 * @brief This file contains the implementation of the utilities for astronomical computations.
 *
 * The functions contained in this file are based in Greg Miller and George H. Kaplan work.
 * You can see more at @a https://www.celestialprogramming.com/
 *
 * @todo Functions will use UTC time, although UT1 should be used.
 *
 * @author    Degoras Project Team.
 * @version   2306.1
 * @date      13-06-2023
 * @copyright EUPL License.
 *
 * @par IMPORTANT NOTE:
 *
 * This code is provided "as is" with no guarantees. The software has been tested with a limited amount of data,
 * and there may still be errors and incomplete implementations.
 *
 **********************************************************************************************************************/

#pragma once

// ========== DPSLR INCLUDES ===========================================================================================
#include "includes/astronomy.h"
#include "includes/dpslr_math.h"
// =====================================================================================================================

// ========== C++ INCLUDES =============================================================================================
#include <cmath>
// =====================================================================================================================

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace astronomy{
// =====================================================================================================================

long double greenwichMeanSiderealTime(long double jd)
{
    long double t = (jd - 2451545.0L) / 36525.L;

    long double gmst = -6.2e-6L*t*t*t + 0.093104L*t*t + (876600.0L * 3600.L + 8640184.812866L)*t + 67310.54841L;  // sec
    //360 deg / 86400 s = 1/240, to convert seconds of time to deg, and then convert to rad
    gmst = dpslr::math::normalizeAngle(gmst / 240.L * dpslr::math::pi / 180.L, 0.L , 2*dpslr::math::pi);

    return gmst;
}


long double localMeanSiderealTime(long double jd, long double lon)
{

    long double gmst = greenwichMeanSiderealTime(jd);
    long double lmst = dpslr::math::normalizeAngle(gmst + lon, 0.L, 2*dpslr::math::pi);

    return lmst;
}

void raDecToAzEl(long double ra, long double dec, long double lmst, long double lat, long double &az, long double &el)
{
    // Local hour angle
    long double lha = lmst - ra;

    el = std::asin(std::sin(dec) * std::sin(lat) + std::cos(dec) * std::cos(lat) * std::cos(lha));
    long double sinv = -std::sin(lha) * std::cos(dec) * std::cos(lat) / (std::cos(el) * std::cos(lat));
    long double cosv = (std::sin(dec) - std::sin(el) * std::sin(lat)) / (std::cos(el) * std::cos(lat));
    az = std::atan2(sinv, cosv);
}

void azElToRaDec(long double az, long double el, long double lmst, long double lat, long double &ra, long double &dec)
{
    dec = std::asin(std::sin(el) * std::sin(lat) + std::cos(el) * std::cos(lat) * std::cos(az));

    long double sinv = -(std::sin(az) * std::cos(el) * std::cos(lat)) / (std::cos(lat) * std::cos(dec));
    long double cosv = (std::sin(el) - std::sin(lat) * std::sin(dec)) / (std::cos(lat) * std::cos(dec));
    long double lha = std::atan2(sinv, cosv);

    ra = lmst - lha;
}

}} // END NAMESPACES
// =====================================================================================================================
