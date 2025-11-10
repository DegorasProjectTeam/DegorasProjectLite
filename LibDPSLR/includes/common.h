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
 * @file common.h
 * @author DEGORAS PROJECT TEAM
 * @date 05-01-2021
 * @brief This file contains several elements that are common to the library (constants, enums, structs, etc.).
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ========== C++ INCLUDES =============================================================================================
#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <algorithm>

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <optional>
#else
#include <experimental/optional>
#endif

#include <fstream>
#include <iomanip>
#include "libdpslr_global.h"
// =====================================================================================================================


// ========== COMMON CONSTANTS =========================================================================================
#if defined(__MINGW32__) || defined(_MSC_VER)
    #define MKGMTIME _mkgmtime
#else
    #define MKGMTIME timegm
#endif
// =====================================================================================================================


// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace  common{
// =====================================================================================================================


// ========== FORWARD DECLARATIONS =====================================================================================
struct LIBDPSLR_EXPORT ConsolidatedRecordStruct;
// =====================================================================================================================


// ========== COMMON ENUMS =============================================================================================

//// @enum ConsolidatedFileTypeEnum
//// This enum represents the different types of ILRS Consolidated files.
enum class ConsolidatedFileTypeEnum
{
    CRD_TYPE,      ///< Consolidated Record Data file.
    CPF_TYPE,      ///< Consolidated Prediction File file.
    UNKNOWN_TYPE   ///< Unknown file.
};

//// @enum RecordReadErrorEnum
//// This enum represents the errors that could happen when a CRD/CPF record (single line) is read.
enum class RecordReadErrorEnum
{
    NOT_ERROR,              ///< No error.
    BAD_SIZE,              ///< Incorrect size errors.
    BAD_TYPE,              ///< Incorrect type errors.
    VERSION_MISMATCH,      ///< Version mismatch
    VERSION_UNKNOWN,       ///< Version unknow (no format header for crd, no basic information for cpf)
    CONVERSION_ERROR,      ///< Conversion error (string to number, string to bool, etc)
    DEPENDENCY_ERROR,      ///< Other struct/data that is necessary for reading the current line is missing.
    OTHER_ERROR,           ///< Other errors  (other exceptions, nullptr pointers, etc)
    NOT_IMPLEMENTED        ///< For not implemented situations.
};

// =====================================================================================================================


// ========== COMMON TYPE ALIAS ========================================================================================

/// Vector that contais pairs. Each pair is pair(time_tag (s), time_of_flight (s)).
using FlightTimeData = std::vector<std::pair<long double, long double>>;

/// Pair(time_tag (s), residual (ps)).
template<typename T = long double, typename R = long double>
using ResidualData = std::pair<T, R>;

/// Vector that residual data
template<typename T = long double, typename R = long double>
using ResidualsData = std::vector<ResidualData<T,R>>;

/// Vector that contais pair of residuals.
template<typename T = long double, typename R = long double>
using ResidualBins = std::vector<ResidualsData<T,R>>;

/// Vector that contains tuples. Each tuple represents: <time_tag (s), tof (ps), pred_dist (ps), trop_corr (ps)>
using RangeData = std::vector<std::tuple<long double, long double, long double, long double>>;

/// Pair(record type enum, RecordLine).
using RecordLinePair = std::pair<int, ConsolidatedRecordStruct>;

/// Pair(record type enum, RecordLine).
using RecordLinesVector = std::vector<ConsolidatedRecordStruct>;

/// Map(record type enum, RecordLine).
using RecordLinesMap = std::map<int, ConsolidatedRecordStruct>;

/// Same than before, but the record type enum can be repeated. Used for example for multiple records like the full
/// rate data or the normal point data.
using RecordLinesMultimap = std::multimap<int, ConsolidatedRecordStruct>;

/// Pair(RecordReadErrorEnum, RecordLine).
using RecordReadErrorPair = std::pair<int, ConsolidatedRecordStruct>;

/// Multimap(RecordReadErrorEnum, RecordLine).
using RecordReadErrorMultimap = std::multimap<int, ConsolidatedRecordStruct>;

/// Time point to store datetimes.
using HRTimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

/// Time point to store datetime.
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

/// Convenient optional (from std::experimental) type alias.
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
template <class T> using optional = std::optional<T>;
#else
template <class T> using optional = std::experimental::optional<T>;
#endif

// =====================================================================================================================


// ========== COMMON CLASSES ===========================================================================================

// --- RECORD BASE CLASS -----------------------------------------------------------------------------------------------
struct ConsolidatedRecordStruct
{
    // Common record lines.
    // -----------------------------------------------------------------------------------------------------------------
    static constexpr const char *CommonRecordsString[] = {"00"};
    // -----------------------------------------------------------------------------------------------------------------

    // Record struct enums.
    // -----------------------------------------------------------------------------------------------------------------
    enum class CommonRecords
    {
        COMMENT_RECORD = 0
    };
    // -----------------------------------------------------------------------------------------------------------------

    // Common members for all the records.
    ConsolidatedFileTypeEnum consolidated_type; // Stores the consolidated type which belongs the record.
    int generic_record_type;                    // For CRD: CRDRecordsTypeEnum    For CPF: CPFRecordsTypeEnum
    std::vector<std::string> comment_block;     // Associated comment bloc (lines "00") for each record.
    std::vector<std::string> tokens;            // For reading files or other usages. ["H1", "CRD", 2, etc]
    optional<unsigned int> line_number;         // Line number in the file, for error handling when reading files.
                                                // Empty value for generated lines (not readed lines).

    inline std::string getIdToken() const {return (tokens.empty() ? "" : tokens[0]);}

    // Function to generatte a comment record block (lines "00").
    std::string generateCommentBlock() const;

    // Clear function.
    void clearAll();
};
// ---------------------------------------------------------------------------------------------------------------------


}} // END NAMESPACES
// =====================================================================================================================




