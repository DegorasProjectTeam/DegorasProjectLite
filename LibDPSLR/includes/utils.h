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
 * @file slrutils.h
 * @brief This file contains several tools (functions and classes) related to SLR.
 * @author DEGORAS PROJECT TEAM
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ========== DP INCLUDES ==============================================================================================
#include "libdpslr_global.h"
#include "common.h"
// =====================================================================================================================


// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace utils{
// =====================================================================================================================

// Constants.
// ---------------------------------------------------------------------------------------------------------------------
constexpr double kModifiedJulianToJulian = 2400000.5;
constexpr double kJulianToModifiedJulian = -2400000.5;
constexpr double kJulianToReducedJulian = -2400000.0;
constexpr double kJulianToJ2000 = -2451545.0;
constexpr double kJ2000ToJulian = 2451545.0;
constexpr double kPosixEpochToJulian = 2440587.5;
constexpr double kJulianToPosixEpoch = -2440587.5;

constexpr long long kNsPerWinTick = 100ULL;


constexpr long long kWin32EpochToPosixEpoch = -11644473600;
// ---------------------------------------------------------------------------------------------------------------------

// Timing conversion helper functions.
// ---------------------------------------------------------------------------------------------------------------------

// ISO 8601 RELATED FUNCTIONS
//======================================================================================================================

// WARNING This functions need a specific test.

std::chrono::seconds iso8601DurationParser(const std::string& duration);

common::HRTimePoint iso8601DatetimeParser(const std::string& datetime);

//======================================================================================================================

// STRING FUNCTIONS
//======================================================================================================================


LIBDPSLR_EXPORT std::string timePointToString(const common::HRTimePoint& tp,
                                              const std::string& format = "%Y-%m-%dT%H:%M:%S",
                                              bool add_ms = true, bool add_ns = false, bool utc = true);

LIBDPSLR_EXPORT std::string timePointToIso8601(const common::HRTimePoint& tp, bool add_ms = true, bool add_ns = false);

LIBDPSLR_EXPORT std::string currentISO8601Date(bool add_ms = true);

LIBDPSLR_EXPORT std::string currentUTCISODate();

//======================================================================================================================

LIBDPSLR_EXPORT common::HRTimePoint winTicksToTimePoint(unsigned long long ticks);

LIBDPSLR_EXPORT common::HRTimePoint win32EpochToTimePoint(long long ns);

LIBDPSLR_EXPORT common::HRTimePoint dateTimeToTimePoint(int y, int m, int d, int h=0, int min=0, int s=0);

LIBDPSLR_EXPORT long double timePointToSecsDay(const dpslr::common::HRTimePoint& tp);

// TLE time (year and fractional point) to C++ time point.
LIBDPSLR_EXPORT dpslr::common::HRTimePoint tleDateToTimePoint(unsigned int cent_year, long double day_with_fract);

// C++ time point to TLE date (year and fractional day).
LIBDPSLR_EXPORT void timePointToTLEDate(const dpslr::common::HRTimePoint& tp, int& cent_year, long double& day_with_fract);

// Transforms hours, mins, seconds and nanoseconds to day nanoseconds.
LIBDPSLR_EXPORT long long hhmmssnsToNsDay(unsigned int hour, unsigned int min, unsigned int sec, unsigned int ns);

// Transforms day nanoseconds to hours, mins, seconds and nanoseconds.
// Returns the days offset as long long.
LIBDPSLR_EXPORT long long nsDayTohhmmssns(long long ns_in, unsigned int& hour_out, unsigned int& min_out,
                                        unsigned int& sec_out, unsigned int& ns_out);

// Transforms the day of the year to the month and day of the month.
// Not works with the 2100, etc year (but we will be dead xD)
LIBDPSLR_EXPORT void ydtomd(int year, unsigned int yday, unsigned int& month, unsigned int& mday);

/**
 * @brief Converts a Gregorian Date to Julian.
 *
 * @param year, Gregorian year.
 * @param month, in range [1,12].
 * @param day, in range [1,31].
 * @param hour, in range [0,23].
 * @param minute, in range [0,59].
 * @param second, in range [0,59].
 * @param jd_day, Julian day. Output.
 * @param jd_fract, Julian fraction of day. Output.
 *
 * @note Not works with the 2100, etc year (but we will be dead xD).
 */
LIBDPSLR_EXPORT void grtojd(int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute,
                          unsigned int sec, long long &jd_day, double &jd_fract);

/**
 * @brief Converts a Julian Date to Gregorian
 *
 * @param jd_day, Julian day
 * @param jd_fract, Julian fraction of day
 * @param year, Gregorian year. Output.
 * @param month, in range [1,12]. Output.
 * @param day, in range [1,31]. Output.
 * @param hour, in range [0,23]. Output.
 * @param minute, in range [0,59]. Output.
 * @param second, in range [0,59]. Output.
 *
 * @note Not works with the 2100, etc year (but we will be dead xD).
 */
LIBDPSLR_EXPORT void jdtogr(long long jd_day, double jd_fract,int &year, unsigned int &month,
                          unsigned int &day, unsigned int &hour, unsigned int &minute, unsigned int &second);

/**
 * @brief Convert a TimePoint to Modified Julian Date
 * @param tp, TimePoint to convert. Input param.
 * @param mjd, Modified Julian day. Output param.
 * @param second_day, Modified Julian second of day. Output param.
 * @param second_fract, Fraction of second. Precission up to ns. Output param.
 */
LIBDPSLR_EXPORT void timePointToModifiedJulianDate(const common::HRTimePoint &tp, unsigned int &mjd,
                                                 unsigned int& second_day, double& second_fract);

/**
 * @brief Converts a TimePoint to Julian Datetime
 * @param tp, TimePoint to convert.
 * @return A floating point value representing Julian Datetime in days. Precission up to ns.
 */
LIBDPSLR_EXPORT long double timePointToJulianDatetime(const dpslr::common::HRTimePoint &tp);

/**
 * @brief Converts a TimePoint to J2000 Datetime
 * @param tp, TimePoint to convert.
 * @return A floating point value representing J2000 Datetime in days. Precission up to ns.
 */
LIBDPSLR_EXPORT long double timePointToJ2000Datetime(const dpslr::common::HRTimePoint &tp);

/**
 * @brief Converts a TimePoint to Modified Julian Datetime
 * @param tp, TimePoint to convert.
 * @return A floating point value representing Modified Julian Datetime in days. Precission up to ns.
 */
LIBDPSLR_EXPORT long double timePointToModifiedJulianDatetime(const dpslr::common::HRTimePoint &tp);

/**
 * @brief Converts a TimePoint to Reduced Julian Datetime
 * @param tp, TimePoint to convert.
 * @return A floating point value representing Reduced Julian Datetime in days. Precission up to ns.
 */
LIBDPSLR_EXPORT long double timePointToReducedJulianDatetime(const dpslr::common::HRTimePoint &tp);

/**
 * @brief Converts a Modified Julian Datetime to a TimePoint.
 * @param mjt, Modified Julian Datetime in days. Precission up to ns. Must be within TimePoint era.
 * @return A TimePoint.
 */
LIBDPSLR_EXPORT dpslr::common::HRTimePoint modifiedJulianDatetimeToTimePoint(long double mjt);

/**
 * @brief Converts a Julian Datetime to a TimePoint.
 * @param mjt, Modified Julian Datetime in days. Precission up to ns. Must be within TimePoint era.
 * @return A TimePoint.
 */
LIBDPSLR_EXPORT dpslr::common::HRTimePoint julianToTimePoint(long double jt);

// ---------------------------------------------------------------------------------------------------------------------

// Satellite identifiers conversion helper functions.
// ---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Converts a COSPAR to a SHORT COSPAR (1986-061A -> 86061A).
 * @param[in] cospar String that represent a COSPAR.
 * @return A string that represent a SHORT COSPAR.
 */
LIBDPSLR_EXPORT std::string cosparToShortcospar(const std::string& cospar);

/**
 * @brief Converts a SHORT COSPAR to an ILRS ID (86061A -> 8606101). This function is not fully compatible with ILRS.
 * @param[in] short_cospar String that represent a SHORT COSPAR.
 * @return A string that represent an ILRS ID.
 */
LIBDPSLR_EXPORT std::string shortcosparToILRSID(const std::string& short_cospar);

/**
 * @brief Converts a COSPAR to an ILRS ID (1986-061A -> 8606101). This function is not fully compatible with ILRS.
 * @param[in] cospar String that represent a COSPAR.
 * @return A string that represent an ILRS ID.
 */
LIBDPSLR_EXPORT std::string cosparToILRSID(const std::string& cospar);

/**
 * @brief Converts a SHORT COSPAR to a COSPAR (86061A -> 1986-061A).
 * @param[in] short_cospar String that represent a SHORT COSPAR.
 * @return A string that represent a COSPAR.
 */
LIBDPSLR_EXPORT std::string shortcosparToCospar(const std::string& short_cospar);

/**
 * @brief Converts an ILRS ID to a SHORT COSPAR (8606101 -> 86061A). This function is not fully compatible with ILRS.
 * @param[in] ilrsid String that represent an ILRS ID.
 * @return A string that represent a SHORT COSPAR.
 */
LIBDPSLR_EXPORT std::string ilrsidToShortcospar(const std::string& ilrsid);

/**
 * @brief Converts an ILRS ID to a COSPAR (8606101 -> 1986-061A). This function is not fully compatible with ILRS.
 * @param[in] ilrsid String that represent an ILRS ID.
 * @return A string that represent a COSPAR.
 */
LIBDPSLR_EXPORT std::string ilrsidToCospar(const std::string& ilrsid);
// ---------------------------------------------------------------------------------------------------------------------

}} // END NAMESPACES.
// =====================================================================================================================
