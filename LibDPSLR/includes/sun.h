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
 * @file sun.h
 *
 * @brief
 *
 * This file contains functions to deal with sun position.
 *
 * @author Degoras Project Tem
 * @copyright EUPL License
 **********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ========== DPSLR INCLUDES ===========================================================================================
#include "libdpslr_global.h"
#include "dpslr_math.h"
#include "common.h"
#include "utils.h"
// =====================================================================================================================

#include <iostream>

namespace dpslr{

namespace sun{

// ========== FUNCTIONS ================================================================================================

// Simple algorithm (VSOP87 algorithm is much more complicated). 0.01 degree accuracy, up to 2099. Only for non scientific purposes.
//    Inspiration from: http ://stjarnhimlen.se/comp/tutorial.html#5
// Book: Sun Position: Astronomical Algorithm in 9 Common Programming Languages
void simpleSunPosition(double j2000, double lat, double lon, bool refr, double& az, double& el)
{
    // Convert latitude and longitude to radians.
    double rlat = math::rad(lat);
    double rlon = math::rad(lon);

    // Local sidereal time.
    long double sidereal = 4.894961213 + 6.300388099 * j2000 + rlon;

    // Mean longitude and anomaly of the sun.
    long double mean_long = j2000 * 1.720279239e-2 + 4.894967873;
    long double mean_anom = j2000 * 1.720197034e-2 + 6.240040768;

    // Ecliptic longitude of the sun.
    long double eclip_long = mean_long + 3.342305518e-2 * std::sin(mean_anom) + 3.490658504e-4 * std::sin(2 * mean_anom);

    // Obliquity of the ecliptic
    long double obliquity = 0.4090877234 - 6.981317008e-9 * j2000;

    // Right ascension of the sun and declination.
    long double rasc = std::atan2(std::cos(obliquity) * std::sin(eclip_long), std::cos(eclip_long));
    long double decl = std::asin(std::sin(obliquity) * std::sin(eclip_long));

    // Hour angle of the sun
    long double hour_ang = sidereal - rasc;

    // Local elevation and azimuth of the sun.
    double elevation = std::asin(std::sin(decl) * std::sin(rlat) + std::cos(decl) *
                                 std::cos(rlat) * std::cos(hour_ang));
    double azimuth = std::atan2(-std::cos(decl) * std::cos(rlat) * std::sin(hour_ang),
                                std::sin(decl) - std::sin(rlat) * std::sin(elevation));

    // Convert azimuth and elevation to degrees and normalize.
    elevation = dpslr::math::normalizeAngle(math::deg(elevation), -180.0, 180.0);
    azimuth = dpslr::math::normalizeAngle(math::deg(azimuth), 0.0, 360.0);

    // Very simple refraction correction (not necessary).
    // Pressure and temperature could be added but for our
    // applications greater precision is not necessary.
    if(refr && (elevation >= -1 * (0.26667 + 0.5667)))
    {
        double targ = math::rad((elevation + (10.3/(elevation + 5.11))));
        elevation += (1.02/tan(targ)) / 60.0; // TODO: Not sure if this is right.
    }

    // Return azimuth and elevation in degrees
    az = azimuth;
    el = elevation;
}

}}


