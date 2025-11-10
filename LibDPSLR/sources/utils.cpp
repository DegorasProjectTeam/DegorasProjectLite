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
 * @file slrutils.cpp
 * @see slrutils.h
 * @author DEGORAS PROJECT TEAM
 * @copyright EUPL License
***********************************************************************************************************************/

// ========== DP INCLUDES ==============================================================================================
#include <includes/helpers.h>
#include <includes/dpslr_math.h>
#include <includes/utils.h>
#include <includes/common.h>
// =====================================================================================================================


// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace utils{
// =====================================================================================================================


long double timePointToSecsDay(const common::HRTimePoint &tp)
{
    std::time_t current = common::HRTimePoint::clock::to_time_t(tp);
    std::tm* current_date = std::gmtime(&current);
    current_date->tm_hour = 0;
    current_date->tm_min = 0;
    current_date->tm_sec = 0;
    current_date->tm_isdst = 0;
    common::HRTimePoint day_start = common::HRTimePoint::clock::from_time_t(MKGMTIME(current_date));
    return std::chrono::duration_cast<std::chrono::duration<long double>>(tp - day_start).count();
}

common::HRTimePoint tleDateToTimePoint(unsigned int cent_year, long double day_with_fract)
{
    using namespace std::chrono;
    auto now_point = system_clock::now();
    std::time_t now = system_clock::to_time_t(now_point);
    std::tm* date = std::gmtime(&now);
    // We set date to last day of the previous year, since
    // day = 1 represents January the 1st, and day = 0 the previous one.
    date->tm_sec = 0;
    date->tm_min = 0;
    date->tm_hour = 0;
    date->tm_mday = 0;
    date->tm_mon = 0;
    // In TLE format, if year is [57,99] it represents 1957-1999, if it is [0,56] it represents 2000-2056
    date->tm_year = cent_year > 56 ? cent_year : cent_year + 100;
    std::time_t start_cent = MKGMTIME(date);
    auto start_cent_point = system_clock::from_time_t(start_cent);
    double iday;
    long double fday = std::modf(day_with_fract, &iday);
    duration<int, std::ratio<86400>> days_duration(static_cast<int>(iday));
    nanoseconds fract_secs_duration(static_cast<long long int>(fday * 86400000000000.0));
    return time_point_cast<common::HRTimePoint::duration>(start_cent_point + days_duration + fract_secs_duration);
}

void timePointToTLEDate(const dpslr::common::HRTimePoint& tp, int& cent_year, long double &day_with_fract)
{
    std::time_t tp_time = std::chrono::system_clock::to_time_t(tp);
    std::tm* date = std::gmtime(&tp_time);
    cent_year = date->tm_year % 100;
    day_with_fract = date->tm_yday + 1;
    date->tm_sec = 0;
    date->tm_min = 0;
    date->tm_hour = 0;
    std::time_t start_day = MKGMTIME(date);
    auto start_day_point = std::chrono::system_clock::from_time_t(start_day);
    std::chrono::duration<double, std::ratio<86400>> day_fract(
              std::chrono::duration_cast<std::chrono::duration<double, std::ratio<86400>>>(tp - start_day_point));
    day_with_fract += day_fract.count();
}

long long hhmmssnsToNsDay(unsigned int hour, unsigned int min, unsigned int sec, unsigned int ns)
{
    // Not use exponential to avoid double.
    return hour * 3600000000000ll + min * 60000000000ll + sec * 1000000000ll + ns;
}

long long nsDayTohhmmssns(long long ns_in, unsigned int& hour, unsigned int& min, unsigned int& sec, unsigned int& ns)
{
    // Not use exponential to avoid double.
    auto result = dpslr::math::euclidDivLL(ns_in, 86400000000000ll);
    ns_in -= result.q * 86400000000000ll;
    hour = ns_in / 3600000000000ll;
    ns_in -= hour * 3600000000000ll;
    min = ns_in / 60000000000ll;
    ns_in -= min * 60000000000ll;
    sec = ns_in / 1000000000ll;
    ns = ns_in % 1000000000ll;
    return  result.q;
}

void ydtomd(int year, unsigned int yday, unsigned int& month, unsigned int& mday)
{
    // Set year to offset since 1900
    year -= 1900;

    // Convert to julian date avoiding leap years by multiplying by 4 and dividing by 1461
    // Set year start two months before to avoid leap day
    double jda1900 = static_cast<long>(1461.0 * (year - 1) / 4.0) + 306.0 + yday;
    int tyear = ((4.0*jda1900) - 1.0) / 1461.0;
    int tday = ((4.0*jda1900) + 3.0 - (tyear*1461.0)) / 4.0;

    month = ((5.0*tday) - 3.0) / 153.0;
    mday = ((5.0*tday) + 2.0 - (153.0*(month))) / 5.0;

    // Set year start to the first month
    if (month >= 10)
        month -= 9;
    else
        month += 3;
}

void grtojd(int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute,
            unsigned int second, long long &jd_day, double &jd_fract)
{
    // Set year to offset since 1900
    year -= 1900;

    // Calculate # days since noon feburary 29, 1900 (julian date=2415078.0)
    if (month <= 2)
        jd_day = static_cast<long long>(1461.0 * (year-1) / 4.0) +
                static_cast<long long>((153.0 * (month+9) + 2.0) / 5.0) + day;
    else
        jd_day = static_cast<long long>(1461.0 * year/4.0) +
                static_cast<long long>((153.0 * (month-3) + 2.0) / 5.0) + day;

    // Add fractional day and the jd for 2/29/1900
    jd_fract = (hour + (minute + second/60.0) / 60.0) / 24.0 + 0.5;
    jd_day += 2415078.0;

    // If jd fract is greater than 1, then add to days and substract to fractional
    long long jd_fract_excess = static_cast<long long>(jd_fract);
    if (jd_fract >= 1.0)
    {
        jd_fract -= jd_fract_excess;
        jd_day += jd_fract_excess;
    }
}

void jdtogr(long long jd_day, double jd_fract,int &year, unsigned int &month, unsigned int &day,
            unsigned int &hour, unsigned int &minute, unsigned int &second)
{
    // Calculate days and fractional part since 1900
    double jdfc = jd_fract + 0.5;
    double jda1900 = jd_day - 2415079.0;

    // Adjust fractional part
    long long jd_fract_excess = static_cast<long long>(jdfc);
    if (jdfc >= 1.0)
    {
        jdfc -= jd_fract_excess;
        jda1900 += jd_fract_excess;
    }

    // Avoid .9999... imprecission
    hour   = jdfc*24.0 + 1.e-10;
    minute = jdfc*1440.0 - hour*60.0 + 1.e-8;
    second = ((jdfc - hour/24.0 - minute/1440.0)*86400.0) + 1.e-8;
    year   = ((4.0*jda1900)-1.0)/1461.0;

    int tday = ((4.0*jda1900) + 3.0 - (year*1461.0))/4.0;
    month = ((5.0*tday) - 3.0) / 153.0;
    day = ((5.0*tday) + 2.0 - (153.0*(month))) / 5.0;

    if (month >= 10)
    {
        month -= 9;
        year++;
    }
    else
    {
        month += 3;
    }

    // Set offset since 1900 to year
    year += 1900;
}

void timePointToModifiedJulianDate(const common::HRTimePoint &tp, unsigned& mjd, unsigned& second_day,
                                   double& second_fract)
{
    using std::chrono::duration;
    using std::chrono::duration_cast;

    double unix_seconds = duration_cast<duration<double>>(tp.time_since_epoch()).count();
    second_fract = unix_seconds - static_cast<long long>(unix_seconds);
    mjd = static_cast<unsigned>((unix_seconds/86400.0) + kPosixEpochToJulian + kJulianToModifiedJulian);
    second_day = static_cast<long long>(unix_seconds) % 86400;
}

long double timePointToJulianDatetime(const dpslr::common::HRTimePoint &tp)
{
    using std::chrono::duration;
    using std::chrono::duration_cast;

    long double unix_seconds = duration_cast<duration<long double>>(tp.time_since_epoch()).count();
    long double jd = (unix_seconds/86400.0) + kPosixEpochToJulian;
    return jd;
}

long double timePointToJ2000Datetime(const dpslr::common::HRTimePoint &tp)
{
    return timePointToJulianDatetime(tp) + kJulianToJ2000;
}

long double timePointToModifiedJulianDatetime(const dpslr::common::HRTimePoint &tp)
{
    return timePointToJulianDatetime(tp) + kJulianToModifiedJulian;
}

long double timePointToReducedJulianDatetime(const dpslr::common::HRTimePoint &tp)
{
    return timePointToJulianDatetime(tp) + kJulianToReducedJulian;
}

dpslr::common::HRTimePoint modifiedJulianDatetimeToTimePoint(long double mjt)
{
    std::chrono::duration<long double, std::ratio<86400>> unix_days(
                mjt + kModifiedJulianToJulian + kJulianToPosixEpoch);
    return dpslr::common::HRTimePoint(std::chrono::duration_cast<dpslr::common::HRTimePoint::duration>(unix_days));
}

dpslr::common::HRTimePoint julianToTimePoint(long double jt)
{
    std::chrono::duration<long double, std::ratio<86400>> unix_days(jt + kJulianToPosixEpoch);
    return dpslr::common::HRTimePoint(std::chrono::duration_cast<dpslr::common::HRTimePoint::duration>(unix_days));
}

std::string cosparToShortcospar(const std::string& cospar)
{
    std::string short_cospar = cospar.substr(2);
    short_cospar.erase(short_cospar.find('-'),1);
    return dpslr::helpers::toUpper(short_cospar);
}

std::string shortcosparToILRSID(const std::string& short_cospar)
{
    // Find the letters.
    auto it = std::find_if(short_cospar.begin(), short_cospar.end(), [](unsigned char a){return std::isalpha(a);});

    // Copy the numbers
    std::string ilrsid(short_cospar.begin(), it);

    // Convert letters to number like this: letter_0_value + letter_1_value * 26 + letter_2_value * 26^2...
    // The letter value is its position within the ASCII alphabet. A = 1, B = 2, etc.
    int num = 0;
    int exp = 0;
    for (auto i = short_cospar.end() - 1; i != (it - 1); i--)
    {
        num += (std::toupper(*i) - 'A' + 1) * std::pow(26, exp);
        exp++;
    }

    // Get only two characters from number
    char buffer[3];
    std::snprintf(buffer, 3, "%02d", num);

    // Return the ILRS ID.
    return ilrsid + buffer;
}

std::string cosparToILRSID(const std::string& cospar)
{
    return shortcosparToILRSID(cosparToShortcospar(cospar));
}

std::string shortcosparToCospar(const std::string& short_cospar)
{
    // Variables.
    std::string cospar;
    std::string year_cent_str = short_cospar.substr(0, 2);
    int year_cent = 0;

    try { year_cent = std::stoi(year_cent_str); } catch (...)
    { return ""; }

    // Since first satellite is in 1957, from 57 to 99, represents 1957 to 1999. 00 to 56 represents 2000 to 2056
    if(year_cent >= 57)
        cospar.append("19");
    else
        cospar.append("20");

    // Append the numbers and letters
    cospar.append(year_cent_str + '-' + short_cospar.substr(2));
    return dpslr::helpers::toUpper(cospar);
}

std::string ilrsidToShortcospar(const std::string& ilrsid)
{
    // The ILRS ID size must be always 7.
    if (ilrsid.size() != 7)
        return {};

    // Convert last two digits to letters. This converssion is the inverse of shortcosparToILRSID
    int num = 0;
    std::string letters;

    try
    {
        num = std::stoi(ilrsid.substr(5));
    }
    catch(...)
    {return {};}

    do
    {
        auto res = dpslr::math::euclidDivLL(num - 1, 26);
        num = res.q;
        letters.push_back('A' + res.r);
    } while(num > 0);

    // Add the letters in correct order to the numbers
    std::string res = ilrsid.substr(0, 5);
    std::reverse_copy(letters.begin(), letters.end(), std::back_inserter(res));

    // Return the short cospar.
    return res;
}

std::string ilrsidToCospar(const std::string& ilrsid)
{
    return shortcosparToCospar(ilrsidToShortcospar(ilrsid));
}

common::HRTimePoint dateTimeToTimePoint(int y, int m, int d, int h, int min, int s)
{
    tm datetime;
    datetime.tm_year = y - 1900;
    datetime.tm_mon = m - 1;
    datetime.tm_mday = d;
    datetime.tm_hour = h;
    datetime.tm_min = min;
    datetime.tm_sec = s;
    return std::chrono::system_clock::from_time_t(MKGMTIME(&datetime));
}

common::HRTimePoint winTicksToTimePoint(unsigned long long ticks)
{
    const unsigned long long ns = ticks * kNsPerWinTick;
    const unsigned long long sec = ns*1e-9;
    const unsigned long long frc = ns % 1000000000ULL;
    auto tp_sec = common::HRTimePoint(std::chrono::seconds(sec+kWin32EpochToPosixEpoch));
    return common::HRTimePoint(tp_sec + std::chrono::nanoseconds(frc));
}

std::string timePointToString(const common::HRTimePoint &tp, const std::string& format,
                              bool add_ms, bool add_ns, bool utc)
{
    // Stream to hold the formatted string and the return container.
    std::ostringstream ss;
    // Convert the time point to a duration and get the different time fractions.
    common::HRTimePoint::duration dur = tp.time_since_epoch();
    const time_t secs = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
    const long long mill = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    const unsigned long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    const unsigned long long s_ns = secs * 1e9;
    const unsigned long long t_ns = (ns - s_ns);
    // Format the duration.
    if (const std::tm *tm = (utc ? std::gmtime(&secs) : std::localtime(&secs)))
    {
        ss << std::put_time(tm, format.c_str());
        if(add_ms && !add_ns)
            ss << '.' << std::setw(3) << std::setfill('0') << (mill - secs * 1e3);
        else if(add_ns)
            ss << '.' << std::setw(9) << std::setfill('0') << t_ns;
    }
    else
    {
        // WARNING TODO Throw exception? If the format is correct this should never happen.
    }
    // Return the container.
    return ss.str();
}

std::string timePointToIso8601(const common::HRTimePoint& tp, bool add_ms, bool add_ns)
{
    // Return the ISO 8601 datetime.
    return timePointToString(tp, "%Y-%m-%dT%H:%M:%S", add_ms, add_ns) + 'Z';
}


std::string currentISO8601Date(bool add_ms)
{
    auto now = std::chrono::high_resolution_clock::now();
    return timePointToIso8601(now, add_ms);
}


}}// END NAMESPACES.
// =====================================================================================================================
