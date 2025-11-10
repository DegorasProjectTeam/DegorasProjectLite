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
 * @file astronomy.h
 *
 * @brief This file contains utilities for astronomical computations.
 *
 * The functions contained in this file are based in David Vallado's implementation.
 *
 * @todo UT1 should be used in JD dates.
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
#include "libdpslr_global.h"
// =====================================================================================================================

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace astronomy{
// =====================================================================================================================

/**
 * @brief Get the Greenwich mean sidereal time as defined in IAU 82.
 * @param jd, Julian date in UT1 in days.
 * @return the Greenwich mean sidereal time in radians (0, 2pi)
 */
long double greenwichMeanSiderealTime(long double jd);

/**
 * @brief Get the Greenwich mean sidereal time.
 * @param jd, Julian date in UT1 in days.
 * @param lon, the longitude of the observer in radians (-2pi to 2pi). West is negative.
 * @return the local sidereal time in radians (0, 2pi)
 */
long double localMeanSiderealTime(long double jd, long double lon);

/**
 * @brief Transform right ascension/declination to azimuth/elevation.
 * @param[in] ra, the right ascension in radians (0, 2pi)
 * @param[in] dec, the declination in radians (-pi/2, pi/2)
 * @param[in] lmst, the local sidereal time
 * @param[in] lat, the latitude of the observer in radians (-pi/2, pi/2)
 * @param[out] az, the resulting azimuth in radians (0, 2pi)
 * @param[out] el, the resulting elevation in radians (-pi/2, pi/2)
 */
void raDecToAzEl(long double ra, long double dec, long double lmst, long double lat, long double &az, long double &el);

/**
 * @brief Transform azimuth/elevation to right ascension/declination.
 * @param[in] az, the azimuth in radians (0, 2pi)
 * @param[in] el, the elevation in radians (-pi/2, pi/2)
 * @param[in] lmst, the local sidereal time
 * @param[in] lat, the latitude of the observer in radians (-pi/2, pi/2)
 * @param[out] ra, the resulting right ascension in radians (0, 2pi)
 * @param[out] dec, the resulting declination in radians (-pi/2, pi/2)
 */
void azElToRaDec(long double az, long double el, long double lmst, long double lat, long double &ra, long double &dec);

}} // END NAMESPACES
// =====================================================================================================================
