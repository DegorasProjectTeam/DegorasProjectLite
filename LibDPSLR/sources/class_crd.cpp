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


// ========== C++ INCLUDES =============================================================================================
#include <fstream>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <cstring>
#include <algorithm>
#include <array>
// =====================================================================================================================


// ========== LOCAL INCLUDES ===========================================================================================
#include "includes/class_crd.h"
#include "includes/dpslr_math.h"
#include "includes/utils.h"
#include "includes/helpers.h"
// =====================================================================================================================


// ========== CRD HEADER ===============================================================================================

// --- CRD HEADER CONST EXPRESSIONS ------------------------------------------------------------------------------------
const std::array<std::string, 5> CRDHeader::HeaderLineString {"H1", "H2", "H3", "H4", "H5"};
const std::array<int, 2> CRDHeader::CRDVersions {1, 2};  // Add new main versions here.
// ---------------------------------------------------------------------------------------------------------------------


// --- CRD HEADER CONSTRUCTORS -----------------------------------------------------------------------------------------
CRDHeader::CRDHeader(float crd_version) :
    format_header(FormatHeader())
{
    this->format_header->crd_version = crd_version;
}

CRDHeader::CRDHeader() :
    format_header(FormatHeader())
{}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD HEADER CLEAR FUNCTIONS --------------------------------------------------------------------------------------
void CRDHeader::clearAll()
{
    // Clear all records
    this->clearFormatHeader();
    this->clearStationHeader();
    this->clearTargetHeader();
    this->clearSessionHeader();
    this->clearPredictionHeader();
}

void CRDHeader::clearFormatHeader() {this->format_header = {};}

void CRDHeader::clearStationHeader() {this->station_header = {};}

void CRDHeader::clearTargetHeader() {this->target_header = {};}

void CRDHeader::clearSessionHeader() {this->session_header = {};}

void CRDHeader::clearPredictionHeader() {this->prediction_header = {};}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD HEADER GETTERS ----------------------------------------------------------------------------------------------
const dpslr::common::optional<CRDHeader::FormatHeader> &CRDHeader::formatHeader() const {return this->format_header;}

const dpslr::common::optional<CRDHeader::StationHeader> &CRDHeader::stationHeader() const {return this->station_header;}

const dpslr::common::optional<CRDHeader::TargetHeader> &CRDHeader::targetHeader() const {return this->target_header;}

const dpslr::common::optional<CRDHeader::SessionHeader> &CRDHeader::sessionHeader() const {return this->session_header;}

const dpslr::common::optional<CRDHeader::PredictionHeader> &CRDHeader::predictionHeader() const
{return this->prediction_header;}

dpslr::common::optional<CRDHeader::FormatHeader> &CRDHeader::formatHeader() {return this->format_header;}

dpslr::common::optional<CRDHeader::StationHeader> &CRDHeader::stationHeader() {return this->station_header;}

dpslr::common::optional<CRDHeader::TargetHeader> &CRDHeader::targetHeader() {return this->target_header;}

dpslr::common::optional<CRDHeader::SessionHeader> &CRDHeader::sessionHeader() {return this->session_header;}

dpslr::common::optional<CRDHeader::PredictionHeader> &CRDHeader::predictionHeader() {return  this->prediction_header;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD HEADER SETTERS ----------------------------------------------------------------------------------------------
void CRDHeader::setStationHeader(const StationHeader &sh) {this->station_header = sh;}

void CRDHeader::setPredictionHeader(const CRDHeader::PredictionHeader& ph) {this->prediction_header = ph;}

void CRDHeader::setFormatHeader(const FormatHeader &fh) {this->format_header = fh;}

void CRDHeader::setSessionHeader(const CRDHeader::SessionHeader &seh) {this->session_header = seh;}

void CRDHeader::setTargetHeader(const CRDHeader::TargetHeader &th) {this->target_header = th;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD HEADER LINES GENERATOR --------------------------------------------------------------------------------------
std::string CRDHeader::generateHeaderLines(bool include_format_header)
{
    // TODO: hacer antes de llamar a esta funcion una funcion de chequeo que compruebe la integridad de esto.
    // Station Network ("na")
    // Target type != 0 y < 5 si v1
    // SIC ("na")
    // if lunar surface SIC and Norad to "na"
    // v1 ending year to "-1" and "na" for v2 -> this is for end sesion data

    // Header line
    std::stringstream header_line;

    // For v1 and 2.
    if(this->format_header->crd_version >= 1 && this->format_header->crd_version <=3)
    {
        // Format header.
        if(include_format_header)
        {
            // Add the associated comment block.
            if(!this->format_header->comment_block.empty())
                header_line << this->format_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->format_header->generateLine() << std::endl;
        }

        // Station header.
        if(this->station_header)
        {
            // Add the associated comment block.
            if(!this->station_header->comment_block.empty())
                header_line << this->station_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->station_header->generateLine(this->format_header->crd_version) << std::endl;
        }

        // Target header.
        if(this->target_header)
        {
            // Add the associated comment block.
            if(!this->target_header->comment_block.empty())
                header_line << this->target_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->target_header->generateLine(this->format_header->crd_version) << std::endl;
        }

        // Session header.
        if(this->session_header)
        {
            // Add the associated comment block.
            if(!this->session_header->comment_block.empty())
                header_line << this->session_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->session_header->generateLine(this->format_header->crd_version) << std::endl;
        }
    }

    // For v2 only.
    if(this->format_header->crd_version >= 2 && this->format_header->crd_version <=3)
    {
        // Prediction header.
        if(this->prediction_header)
        {
            // Add the associated comment block.
            if(!this->prediction_header->comment_block.empty())
                header_line << this->prediction_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->prediction_header->generateLine(this->format_header->crd_version) << std::endl;
        }
    }

    // Return all the lines.
    return header_line.str().substr(0, header_line.str().find_last_of('\n'));
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD HEADER PUBLIC READER FUNCTIONS ------------------------------------------------------------------------------

// Generic reading header function.
dpslr::common::RecordReadErrorMultimap CRDHeader::readHeader(const dpslr::common::RecordLinesVector &rec_v)
{
    // Aux variables.
    dpslr::common::RecordLinePair rec_pair;
    dpslr::common::RecordReadErrorMultimap error_map;
    dpslr::common::RecordReadErrorEnum error;
    int pos;

    // First, clear the data.
    this->clearAll();

    // Read the line and store the error, if any.
    for (const auto& rec : rec_v)
    {
        // Check that the record is a header record.
        if(dpslr::helpers::find(HeaderLineString, rec.getIdToken(), pos))
        {
            // Store the record type in a pair.
            rec_pair = {pos, rec};

            // Read the record to store the data in memory.
            error = this->readHeaderLine(rec_pair);

            // Check for errors.
            if(error != dpslr::common::RecordReadErrorEnum::NOT_ERROR)
                error_map.emplace(static_cast<int>(error), rec);
        }
        else
        {
            error_map.emplace(static_cast<int>(dpslr::common::RecordReadErrorEnum::BAD_TYPE), rec);
        }
    }
    // Return the map with the errors. If no errors, the map will be empty.
    return error_map;
}

// Specific function for reading H1.
dpslr::common::RecordReadErrorEnum CRDHeader::readFormatHeader(const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens are: H1, CRD, VERSION, YEAR, MONTH, DAY, HOUR
    // This read MUST be ok, because we always need the version for reading the next data.

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Variables.
    std::tm date_time;
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    std::string aux = dpslr::helpers::toUpper(tokens[0]);

    // Check if size is correct.
    if (tokens.size() != 7)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != HeaderLineString[static_cast<int>(HeaderRecordEnum::FORMAT_HEADER)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    else
    {
        try
        {
            // New format header struct.
            FormatHeader fh;

            // Get the line and version.
            fh.crd_version = std::stof(tokens[2]);

            auto it = std::find(CRDVersions.begin(), CRDVersions.end(), static_cast<int>(fh.crd_version));
            if(it == CRDVersions.end())
            {
                result = dpslr::common::RecordReadErrorEnum::VERSION_MISMATCH;
            }
            else
            {
                // Get the file creation time UTC.
                date_time.tm_year = std::stoi(tokens[3]) - 1900;
                date_time.tm_mon = std::stoi(tokens[4]) - 1;
                date_time.tm_mday = std::stoi(tokens[5]);
                date_time.tm_hour = std::stoi(tokens[6]);
                date_time.tm_min = 0;
                date_time.tm_sec = 0;
                date_time.tm_isdst = 0;
                fh.crd_production_date = std::chrono::system_clock::from_time_t(MKGMTIME(&date_time));

                // Add the associated comments, the line number, and the tokens.
                fh.comment_block = record.comment_block;
                fh.line_number = record.line_number;
                fh.tokens = record.tokens;

                // Finally, store header if no converssion error ocurred
                this->format_header = std::move(fh);
            }

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }
    }

    // If there was any error, clear format header
    if (dpslr::common::RecordReadErrorEnum::NOT_ERROR != result)
        this->clearFormatHeader();

    // Return the result.
    return result;
}

// Specific function for reading H2.
dpslr::common::RecordReadErrorEnum CRDHeader::readStationHeader(const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens are: H2, NAME, SYSTEM ID, SYSTEM NUMBER, SYSTEM OCCUPANCY, TIME SCALE, [STATION NETWORK]

    // Delete the current data.
    this->clearStationHeader();

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check if format header is ok
    if(!this->format_header)
        return dpslr::common::RecordReadErrorEnum::VERSION_UNKNOWN;

    // Check the data size for each version.
    // For v1.
    if (this->format_header->crd_version >= 1 && this->format_header->crd_version < 2 && tokens.size() != 6)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // For v2.
    else if(this->format_header->crd_version >= 2 && this->format_header->crd_version < 3 && tokens.size() != 7)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != HeaderLineString[static_cast<int>(HeaderRecordEnum::STATION_HEADER)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    try
    {
        // New station header struct.
        StationHeader sh;

        // Get the data.
        sh.official_name = tokens[1];
        sh.cdp_pad_identifier = std::stoi(tokens[2]);
        sh.cdp_system_number = std::stoi(tokens[3]);
        sh.cdp_occupancy_sequence = std::stoi(tokens[4]);
        sh.epoch_timescale = static_cast<TimeScaleEnum>(std::stoi(tokens[5]));

        // Only for v2.
        if(this->format_header->crd_version >= 2 && this->format_header->crd_version < 3)
            sh.network = tokens[6];

        // Add the associated comments, the line number, and the tokens.
        sh.comment_block = record.comment_block;
        sh.line_number = record.line_number;
        sh.tokens = record.tokens;

        // If there was no error at reading, store header
        this->station_header = std::move(sh);

    } catch (...)
    {
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

// Specific function for reading H3.
dpslr::common::RecordReadErrorEnum CRDHeader::readTargetHeader(const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens are: H3, NAME, ILRSID, SIC, NORAD, EPOCH TIME SCALE, [CLASS], [LOCATION]

    // Delete the current data.
    this->clearTargetHeader();

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check if format header is ok
    if(!this->format_header)
        return dpslr::common::RecordReadErrorEnum::VERSION_UNKNOWN;

    // Check the data size for each version.
    // For v1.
    if (this->format_header->crd_version >= 1 && this->format_header->crd_version < 2 && tokens.size() != 7)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // For v2.
    else if(this->format_header->crd_version >= 2 && this->format_header->crd_version < 3 && tokens.size() != 8)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != HeaderLineString[static_cast<int>(HeaderRecordEnum::TARGET_HEADER)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    try
    {
        // New station header struct.
        TargetHeader th;

        // Get the data.
        th.name = tokens[1];

        // Store the target id.
        std::string target_id = tokens[2];

        // We suppose that is a full COSPAR.
        if(target_id.find('-') != std::string::npos)
        {
            th.cospar = target_id;
            th.ilrsid = dpslr::utils::cosparToILRSID(target_id);
        }
        // We suppose that is SHORT COSPAR.
        else if(std::find_if(target_id.begin(), target_id.end(), ::isalpha) != target_id.end())
        {
            th.cospar = dpslr::utils::shortcosparToCospar(target_id);
            th.ilrsid = dpslr::utils::shortcosparToILRSID(target_id);
        }
        // We suppose that is a ILRS ID.
        else if(target_id.size() == 7)
        {
            th.cospar = dpslr::utils::ilrsidToShortcospar(target_id);
            th.ilrsid = target_id;
        }
        else
            throw std::runtime_error(target_id);

        // Rest of the data.
        th.sic = tokens[3];
        th.norad = tokens[4];
        th.sc_epoch_ts = static_cast<SpacecraftEpochTimeScaleEnum>(std::stoi(tokens[5]));
        th.target_class = static_cast<TargetClassEnum>(std::stoi(tokens[6]));

        // Only for v2.
        if(this->format_header->crd_version >= 2 && this->format_header->crd_version < 3)
        {
            th.location = static_cast<TargetLocationEnum>(std::stoi(tokens[7]));
        }

        // Add the associated comments, the line number, and the tokens.
        th.comment_block = record.comment_block;
        th.line_number = record.line_number;
        th.tokens = record.tokens;

        // If there was no error at reading, store header
        this->target_header = std::move(th);

    } catch (...)
    {
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

// Specific function for reading H4.
dpslr::common::RecordReadErrorEnum CRDHeader::readSessionHeader(const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens are: H4, TYPE, SYY, SMM, SDD, SHH, Smm, Sss, EYY, EMM, EDD, EHH, Emm, Ess, RELEASE, TROP_FLAG,
    //             MASS_FLAG, AMP, STATION_DELAY_FLAG, SPACECRAFT_DELAY_FLAG, RANGE_TYPE, QUALITY

    // Delete the current data.
    this->clearSessionHeader();

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Variables.
    std::tm start_time;
    std::tm end_time;

    // Check if size is correct.
    if (tokens.size() != 22)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != HeaderLineString[static_cast<int>(HeaderRecordEnum::SESSION_HEADER)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    try
    {
        // New session header struct
        SessionHeader sh;

        // Get data type.
        sh.data_type = static_cast<DataTypeEnum>(std::stoi(tokens[1]));

        // Get the session start time UTC.
        start_time.tm_year = std::stoi(tokens[2]) - 1900;
        start_time.tm_mon = std::stoi(tokens[3]) - 1;
        start_time.tm_mday = std::stoi(tokens[4]);
        start_time.tm_hour = std::stoi(tokens[5]);
        start_time.tm_min = std::stoi(tokens[6]);
        start_time.tm_sec = std::stoi(tokens[7]);
        start_time.tm_isdst = 0;
        sh.start_time = std::chrono::system_clock::from_time_t(MKGMTIME(&start_time));

        // Get the session end time UTC.
        end_time.tm_year = std::stoi(tokens[8]) - 1900;
        end_time.tm_mon = std::stoi(tokens[9]) - 1;
        end_time.tm_mday = std::stoi(tokens[10]);
        end_time.tm_hour = std::stoi(tokens[11]);
        end_time.tm_min = std::stoi(tokens[12]);
        end_time.tm_sec = std::stoi(tokens[13]);
        end_time.tm_isdst = 0;
        sh.end_time = std::chrono::system_clock::from_time_t(MKGMTIME(&end_time));

        // Get the rest of the data.
        sh.data_release = std::stoi(tokens[14]);
        sh.trop_correction_applied = dpslr::helpers::BoolString(tokens[15]);
        sh.com_correction_applied = dpslr::helpers::BoolString(tokens[16]);
        sh.rcv_amp_correction_applied = dpslr::helpers::BoolString(tokens[17]);
        sh.stat_delay_applied = dpslr::helpers::BoolString(tokens[18]);
        sh.spcraft_delay_applied = dpslr::helpers::BoolString(tokens[19]);
        sh.range_type = static_cast<RangeTypeEnum>(std::stoi(tokens[20]));
        sh.data_quality_alert = static_cast<DataQualityEnum>(std::stoi(tokens[21]));

        // Add the associated comments.
        sh.comment_block = record.comment_block;
        sh.line_number = record.line_number;
        sh.tokens = record.tokens;

        // If there was no error at reading, store header
        this->session_header = std::move(sh);

    } catch (...)
    {
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

// Specific function for reading H5.
dpslr::common::RecordReadErrorEnum CRDHeader::readPredictionHeader(const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens are: H5, PRED_TYPE, YEAR_CENTURY_CPF_TLE, DATE_TIME, PRED_PROV, SEQ_NUMBER

    // Clear current data.
    this->clearPredictionHeader();

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check if size is correct.
    if (tokens.size() != 6)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != HeaderLineString[static_cast<int>(HeaderRecordEnum::PREDICTION_HEADER)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    try
    {
        // New session header struct.
        PredictionHeader ph;

        // Get prediction type.
        ph.prediction_type = static_cast<PredictionTypeEnum>(std::stoi(tokens[1]));

        // Get year of century
        int year = std::stoi(tokens[2]);

        // Get prediction file creation time
        if (ph.prediction_type == PredictionTypeEnum::CPF)
        {
            // TODO: change this by std::get_time whenever it is available in gcc
            int month = std::stoi(tokens[3].substr(0,2));
            int day = std::stoi(tokens[3].substr(2,2));
            int hour = std::stoi(tokens[3].substr(4,2));
            std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm* date = std::gmtime(&now);
            // Set timepoint at current century
            date->tm_year -= date->tm_year % 100;
            date->tm_year += year;
            date->tm_mon = month - 1;
            date->tm_mday = day;
            date->tm_hour = hour;
            date->tm_min = 0;
            date->tm_sec = 0;
            std::time_t prod_date = MKGMTIME(date);
            ph.datetime = std::chrono::system_clock::from_time_t(prod_date);
        }
        else if (ph.prediction_type == PredictionTypeEnum::TLE)
        {
            // Get day with fractional part
            double day = std::stod(tokens[3]);
            ph.datetime = dpslr::utils::tleDateToTimePoint(year, day);
        }
        else
        {
            return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
        }

        // Get prediction provider and sequence number.
        ph.prediction_provider = tokens[4];
        ph.sequence_number = std::stoi(tokens[5]);

        // Add the associated comments, the line number, and the tokens.
        ph.comment_block = record.comment_block;
        ph.line_number = record.line_number;
        ph.tokens = record.tokens;

        // If there was no error at reading, store header
        this->prediction_header = std::move(ph);

    } catch (...)
    {
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}


// --- CRD HEADER PRIVATE FUNCTIONS  -----------------------------------------------------------------------------------
dpslr::common::RecordReadErrorEnum CRDHeader::readHeaderLine(const dpslr::common::RecordLinePair &lpair)
{ 
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;

    // Select the specific read funtion for each line.
    switch (static_cast<HeaderRecordEnum>(lpair.first))
    {
        case HeaderRecordEnum::FORMAT_HEADER:
        result = this->readFormatHeader(lpair.second);
        break;

        case HeaderRecordEnum::STATION_HEADER:
        result = this->readStationHeader(lpair.second);
        break;

        case HeaderRecordEnum::TARGET_HEADER:
        result = this->readTargetHeader(lpair.second);
        break;

        case HeaderRecordEnum::SESSION_HEADER:
        result = this->readSessionHeader(lpair.second);
        break;

        case HeaderRecordEnum::PREDICTION_HEADER:
        result = this->readPredictionHeader(lpair.second);
        break;
    }

    return result;
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD HEADERS STRUCTS ---------------------------------------------------------------------------------------------
std::string CRDHeader::FormatHeader::generateLine()
{
    // Base line.
    std::stringstream line_h1;

    // Get the creation time (UTC).
    std::tm *time;
    dpslr::common::HRTimePoint timepoint = dpslr::common::HRTimePoint::clock::now();
    std::time_t datetime = std::chrono::system_clock::to_time_t(timepoint);
    time = std::gmtime(&datetime);

    // Update the production date in the class.
    this->crd_production_date = timepoint;

    // Generate H1 for version 1 (fixed format).
    if (this->crd_version >= 1 && this->crd_version < 2)
    {
        line_h1 << std::fixed << std::right
                << "H1 CRD"
                << std::setw(3) << static_cast<int>(this->crd_version)
                << std::setw(5) << (time->tm_year + 1900)
                << std::setw(3) << (time->tm_mon + 1)
                << std::setw(3) << (time->tm_mday)
                << std::setw(3) << (time->tm_hour);
    }

    // Generate H1 for version 2 (free format).
    if (this->crd_version >= 2  && this->crd_version < 3)
    {
        line_h1 << "H1 CRD"
                << ' ' << this->crd_version          // Free format, so we could print "2.0", "2.1", etc.
                << ' ' << (time->tm_year + 1900)
                << ' ' << (time->tm_mon + 1)
                << ' ' << (time->tm_mday)
                << ' ' << (time->tm_hour);
    }

    // Return the H1
    return line_h1.str();
}

std::string CRDHeader::StationHeader::generateLine(float version) const
{
    // H2 Base line.
    std::stringstream line_h2;

    // For version 1 and 2 (fixed format).
    if (version >= 1 && version < 2)
    {
        line_h2 << std::fixed << std::left
                << "H2 "
                << std::setw(10) << this->official_name.substr(0, 10)
                << std::right
                << std::setw(5)  << this->cdp_pad_identifier
                << std::setw(3)  << this->cdp_system_number
                << std::setw(3)  << this->cdp_occupancy_sequence
                << std::setw(3)  << static_cast<int>(this->epoch_timescale);
    }

    // For v2 only (free format).
    if(version >= 2 && version < 3)
    {
        line_h2 << std::fixed
                << "H2"
                << ' ' << this->official_name
                << ' ' << this->cdp_pad_identifier
                << ' ' << this->cdp_system_number
                << ' ' << this->cdp_occupancy_sequence
                << ' ' << static_cast<int>(this->epoch_timescale)
                << ' ' << (this->network.empty() ? "na" : this->network);
    }

    // Return the H2
    return line_h2.str();
}

std::string CRDHeader::TargetHeader::generateLine(float version) const
{
    // H3 Base line.
    std::stringstream line_h3;

    // For version 1 (fixed format).
    if (version >= 1 && version < 2)
    {
        // TODO: name should be lowercase only in ILRS or always?
        line_h3 << std::fixed << std::left
                << "H3 "
                << std::setw(10) << dpslr::helpers::toLower(this->name.substr(0, 10))
                << std::right
                << std::setw(9)  << this->ilrsid.substr(0, 8)
                << std::setw(5)  << (this->sic.empty() ? "9999" : this->sic.substr(0, 4))
                << std::setw(9)  << (this->norad.empty() ? "99999999" : this->norad.substr(0, 8))
                << std::setw(2)  << static_cast<int>(this->sc_epoch_ts)
                << std::setw(2)  << static_cast<int>(this->target_class);
    }

    // For v2 only (free format).
    if(version >= 2 && version < 3)
    {
        line_h3 << std::fixed
                << "H3"
                << ' ' << dpslr::helpers::toLower(this->name)
                << ' ' << this->ilrsid
                << ' ' << (this->sic.empty() ? "na" : this->sic)
                << ' ' << (this->norad.empty() ? "na" : this->norad)
                << ' ' << static_cast<int>(this->sc_epoch_ts)
                << ' ' << static_cast<int>(this->target_class)
                << ' ' << (this->location == TargetLocationEnum::UNKNOWN_LOCATION ?
                               "na" : std::to_string(static_cast<int>(this->location)));
    }

    // Return the H3
    return line_h3.str();
}

std::string CRDHeader::SessionHeader::generateLine(float version) const
{
    // H4 Base line
    std::stringstream line_h4;

    // Get the session times.
    std::time_t start = std::chrono::system_clock::to_time_t(this->start_time);
    std::time_t end = std::chrono::system_clock::to_time_t(this->end_time);
    std::tm start_tm(*std::gmtime(&start));
    std::tm end_tm(*std::gmtime(&end));

    // For version 1 (fixed format).
    if (version >= 1 && version < 2)
    {
        line_h4 << std::fixed << std::right
                << "H4"
                << std::setw(3) << static_cast<int>(this->data_type)
                << std::setw(5) << (start_tm.tm_year + 1900)
                << std::setw(3) << (start_tm.tm_mon + 1)
                << std::setw(3) << (start_tm.tm_mday)
                << std::setw(3) << (start_tm.tm_hour)
                << std::setw(3) << (start_tm.tm_min)
                << std::setw(3) << (start_tm.tm_sec)
                << std::setw(5) << (end_tm.tm_year + 1900)
                << std::setw(3) << (end_tm.tm_mon + 1)
                << std::setw(3) << (end_tm.tm_mday)
                << std::setw(3) << (end_tm.tm_hour)
                << std::setw(3) << (end_tm.tm_min)
                << std::setw(3) << (end_tm.tm_sec)
                << std::setw(3) << this->data_release
                << std::setw(2) << this->trop_correction_applied
                << std::setw(2) << this->com_correction_applied
                << std::setw(2) << this->rcv_amp_correction_applied
                << std::setw(2) << this->stat_delay_applied
                << std::setw(2) << this->spcraft_delay_applied
                << std::setw(2) << static_cast<int>(this->range_type)
                << std::setw(2) << static_cast<int>(this->data_quality_alert);
    }

    // For version 2 (free format).
    if (version >= 2 && version < 3)
    {
        line_h4 << std::fixed << std::left
                << "H4"
                << ' ' << static_cast<int>(this->data_type)
                << ' ' << (start_tm.tm_year + 1900)
                << ' ' << (start_tm.tm_mon + 1)
                << ' ' << (start_tm.tm_mday)
                << ' ' << (start_tm.tm_hour)
                << ' ' << (start_tm.tm_min)
                << ' ' << (start_tm.tm_sec)
                << ' ' << (end_tm.tm_year + 1900)
                << ' ' << (end_tm.tm_mon + 1)
                << ' ' << (end_tm.tm_mday)
                << ' ' << (end_tm.tm_hour)
                << ' ' << (end_tm.tm_min)
                << ' ' << (end_tm.tm_sec)
                << ' ' << this->data_release
                << ' ' << this->trop_correction_applied
                << ' ' << this->com_correction_applied
                << ' ' << this->rcv_amp_correction_applied
                << ' ' << this->stat_delay_applied
                << ' ' << this->spcraft_delay_applied
                << ' ' << static_cast<int>(this->range_type)
                << ' ' << static_cast<int>(this->data_quality_alert);
    }

    // Return the H4
    return line_h4.str();
}

std::string CRDHeader::PredictionHeader::generateLine(float version) const
{
    // H5 Base line.
    std::stringstream line_h5;

    // For v2 only (free format).
    if(version >= 2 && version < 3)
    {
        // Prediction type.
        line_h5 << std::fixed
                << "H5"
                << ' ' << static_cast<int>(this->prediction_type);

        if (this->prediction_type == PredictionTypeEnum::CPF)
        {
            // Get prediction file creation time.
            std::time_t creation = std::chrono::system_clock::to_time_t(this->datetime);
            std::tm creation_tm(*std::gmtime(&creation));

            // Store the data.
            line_h5 << ' ' << creation_tm.tm_year % 100 << ' ' << std::setfill('0')
                    << std::setw(2) << creation_tm.tm_mon + 1
                    << std::setw(2) << creation_tm.tm_mday
                    << std::setw(2) << creation_tm.tm_hour;
        }
        else if (this->prediction_type == PredictionTypeEnum::TLE)
        {
            // Aux variables.
            int year;
            long double fractional;

            // Calculate the fractional day.
            dpslr::utils::timePointToTLEDate(this->datetime, year, fractional);

            // Store the data.
            line_h5 << std::setprecision(11)
                    << ' ' << year
                    << ' ' << fractional;
        }

        // Provider and sequence number.
        line_h5 << ' ' << this->prediction_provider
                << ' ' << this->sequence_number;
    }

    // Return the H5
    return line_h5.str();
}
// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================


// ========== CRD CONFIGURATION ========================================================================================

// --- CRD CONFIGURATION CONST EXPRESSIONS -----------------------------------------------------------------------------
const std::array<const char*, 8> CRDConfiguration::CfgLineString {"C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7"};
// ---------------------------------------------------------------------------------------------------------------------


// --- CRD HEADER CLEAR FUNCTIONS --------------------------------------------------------------------------------------
void CRDConfiguration::clearAll()
{
    this->clearSystemConfiguration();
    this->clearLaserConfiguration();
    this->clearDetectorConfiguration();
    this->clearTimingConfiguration();
    this->clearTransponderConfiguration();
    this->clearSoftwareConfiguration();
    this->clearMeteorologicalConfiguration();
    this->clearCalibrationTargetConfiguration();
}

void CRDConfiguration::clearSystemConfiguration() {this->system_cfg = {};}

void CRDConfiguration::clearLaserConfiguration(){this->laser_cfg = {};}

void CRDConfiguration::clearDetectorConfiguration() {this->detector_cfg = {};}

void CRDConfiguration::clearTimingConfiguration() {this->timing_cfg = {};}

void CRDConfiguration::clearTransponderConfiguration(){this->transponder_cfg = {};}

void CRDConfiguration::clearSoftwareConfiguration() {this->software_cfg = {};}

void CRDConfiguration::clearMeteorologicalConfiguration() {this->meteorological_cfg = {};}

void CRDConfiguration::clearCalibrationTargetConfiguration() {this->calib_target_cfg = {};}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CONFIGURATION GETTERS ---------------------------------------------------------------------------------------
const dpslr::common::optional<CRDConfiguration::SystemConfiguration> &CRDConfiguration::systemConfiguration() const
{return this->system_cfg;}

const dpslr::common::optional<CRDConfiguration::LaserConfiguration> &CRDConfiguration::laserConfiguration() const
{return this->laser_cfg;}

const dpslr::common::optional<CRDConfiguration::DetectorConfiguration> &CRDConfiguration::detectorConfiguration() const
{return this->detector_cfg;}

const dpslr::common::optional<CRDConfiguration::TimingConfiguration> &CRDConfiguration::timingConfiguration() const
{return this->timing_cfg;}

const dpslr::common::optional<CRDConfiguration::TransponderConfiguration> &CRDConfiguration::transponderConfiguration()
const {return this->transponder_cfg;}

const dpslr::common::optional<CRDConfiguration::SoftwareConfiguration> &CRDConfiguration::softwareConfiguration() const
{return this->software_cfg;}

const dpslr::common::optional<CRDConfiguration::MeteorologicalConfiguration>
&CRDConfiguration::meteorologicalConfiguration() const {return  this->meteorological_cfg;}

const dpslr::common::optional<CRDConfiguration::CalibrationTargetConfiguration>
&CRDConfiguration::calibrationConfiguration() const {return  this->calib_target_cfg;}

dpslr::common::optional<CRDConfiguration::SystemConfiguration> &CRDConfiguration::systemConfiguration()
{return this->system_cfg;}

dpslr::common::optional<CRDConfiguration::LaserConfiguration> &CRDConfiguration::laserConfiguration()
{return this->laser_cfg;}

dpslr::common::optional<CRDConfiguration::DetectorConfiguration> &CRDConfiguration::detectorConfiguration()
{return this->detector_cfg;}

dpslr::common::optional<CRDConfiguration::TimingConfiguration> &CRDConfiguration::timingConfiguration()
{return this->timing_cfg;}

dpslr::common::optional<CRDConfiguration::TransponderConfiguration> &CRDConfiguration::transponderConfiguration()
{return this->transponder_cfg;}

dpslr::common::optional<CRDConfiguration::SoftwareConfiguration> &CRDConfiguration::softwareConfiguration()
{return this->software_cfg;}

dpslr::common::optional<CRDConfiguration::MeteorologicalConfiguration> &CRDConfiguration::meteorologicalConfiguration()
{return this->meteorological_cfg;}

dpslr::common::optional<CRDConfiguration::CalibrationTargetConfiguration> &CRDConfiguration::calibrationConfiguration()
{return this->calib_target_cfg;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CONFIGURATION SETTERS ---------------------------------------------------------------------------------------
void CRDConfiguration::setSystemConfiguration(const CRDConfiguration::SystemConfiguration& sc) {this->system_cfg = sc;}

void CRDConfiguration::setLaserConfiguration(const CRDConfiguration::LaserConfiguration& lc) {this->laser_cfg = lc;}

void CRDConfiguration::setDetectorConfiguration(const CRDConfiguration::DetectorConfiguration& dc)
{this->detector_cfg = dc;}

void CRDConfiguration::setTimingConfiguration(const CRDConfiguration::TimingConfiguration& tc) {this->timing_cfg = tc;}

void CRDConfiguration::setTransponderConfiguration(const CRDConfiguration::TransponderConfiguration& tc)
{this->transponder_cfg = tc;}

void CRDConfiguration::setSoftwareConfiguration(const CRDConfiguration::SoftwareConfiguration& sc)
{this->software_cfg = sc;}

void CRDConfiguration::setMeteorologicalConfiguration(const CRDConfiguration::MeteorologicalConfiguration& mc)
{this->meteorological_cfg = mc;}

void CRDConfiguration::setCalibrationTargetConfiguration(const CRDConfiguration::CalibrationTargetConfiguration& cc)
{this->calib_target_cfg = cc;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CONFIGURATION LINES GENERATOR -------------------------------------------------------------------------------
std::string CRDConfiguration::generateConfigurationLines(float version) const
{
    // Header line
    std::stringstream cfg_lines;

    // For v1 and 2.
    if(version >= 1 && version <=3)
    {
        // Add the system cfg comment block.
        if(!this->system_cfg->comment_block.empty())
            cfg_lines << this->system_cfg->generateCommentBlock() << std::endl;

        cfg_lines << this->generateSystemCfgLine(version) << std::endl;

        if(this->laser_cfg)
        {
            // Add the laser cfg comment block.
            if(!this->laser_cfg->comment_block.empty())
                cfg_lines << this->laser_cfg->generateCommentBlock() << std::endl;

            cfg_lines << this->laser_cfg->generateLine(version) << std::endl;
        }

        if(this->detector_cfg)
        {
            // Add the detector cfg comment block.
            if(!this->detector_cfg->comment_block.empty())
                cfg_lines << this->detector_cfg->generateCommentBlock() << std::endl;

            cfg_lines << this->detector_cfg->generateLine(version) << std::endl;
        }

        if(this->timing_cfg)
        {
            // Add the timing cfg comment block.
            if(!this->timing_cfg->comment_block.empty())
                cfg_lines << this->timing_cfg->generateCommentBlock() << std::endl;

            cfg_lines << this->timing_cfg->generateLine(version) << std::endl;
        }

        if(this->transponder_cfg)
        {
            // Add the transponder cfg comment block.
            if(!this->transponder_cfg->comment_block.empty())
                cfg_lines << this->transponder_cfg->generateCommentBlock() << std::endl;

            cfg_lines << this->transponder_cfg->generateLine(version) << std::endl;
        }
    }

    // For v2 only.
    if(version >= 2 && version <=3)
    {
        if(this->software_cfg)
        {
            // Add the system cfg comment block.
            if(!this->software_cfg->comment_block.empty())
                cfg_lines << this->software_cfg->generateCommentBlock() << std::endl;

            cfg_lines << this->software_cfg->generateLine(version) << std::endl;
        }

        if(this->meteorological_cfg)
        {
            // Add the meteo cfg comment block.
            if(!this->meteorological_cfg->comment_block.empty())
                cfg_lines << this->meteorological_cfg->generateCommentBlock() << std::endl;

            cfg_lines << this->meteorological_cfg->generateLine(version) << std::endl;
        }

        if(this->calib_target_cfg)
        {
            // Add the calibration cfg comment block.
            if(!this->calib_target_cfg->comment_block.empty())
                cfg_lines << this->calib_target_cfg->generateCommentBlock() << std::endl;

            cfg_lines << this->calib_target_cfg->generateLine(version) << std::endl;
        }
    }

    return cfg_lines.str().substr(0, cfg_lines.str().find_last_of('\n'));
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CONFIGURATION PUBLIC READER FUNCTIONS -----------------------------------------------------------------------
// Generic reading configuration function.
dpslr::common::RecordReadErrorMultimap CRDConfiguration::readConfiguration(const dpslr::common::RecordLinesVector &rec_v, float v)
{
    // Aux variables.
    dpslr::common::RecordLinePair rec_pair;
    dpslr::common::RecordReadErrorMultimap error_map;
    dpslr::common::RecordReadErrorEnum error;
    int pos;

    // First, clear the data.
    this->clearAll();

    // Read the line and store the error, if any.
    for (const auto& rec : rec_v)
    {
        // Check that the record is a header record.
        if(dpslr::helpers::find(CfgLineString, rec.getIdToken(), pos))
        {
            // Store the record type in a pair.
            rec_pair = {pos, rec};

            // Read the record to store the data in memory.
            error = this->readConfigurationLine(rec_pair, v);

            // Check for errors.
            if(error != dpslr::common::RecordReadErrorEnum::NOT_ERROR)
                error_map.emplace(static_cast<int>(error), rec);
        }
        else
        {
            error_map.emplace(static_cast<int>(dpslr::common::RecordReadErrorEnum::BAD_TYPE), rec);
        }
    }

    // Return the map with the errors. If no errors, the map will be empty.
    return error_map;
}

// Specific function for reading C0.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readSystemCFG(
        const dpslr::common::ConsolidatedRecordStruct& record, float)
{
    // Variables.
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    // Tokens are: C0, DETAIL, TRANSMIT WAVE, CFG ID, A, B, C, D, E, F, G
    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Delete the current data.
    this->clearSystemConfiguration();

    // Check if size is correct.
    if (tokens.size() < 7 || tokens.size() > 11)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != CfgLineString[static_cast<int>(ConfigurationLineEnum::SYSTEM_CFG)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    else
        try
        {
            // New system configuration struct.
            SystemConfiguration sc;

            // Get the data.
            sc.detail_type = std::stoi(tokens[1]);
            sc.transmit_wavelength = std::stod(tokens[2]);
            sc.system_cfg_id = tokens[3];

            // Add the associated comments, the line number, and the tokens.
            sc.comment_block = record.comment_block;
            sc.line_number = record.line_number;
            sc.tokens = record.tokens;

            // Store the system configuration struct
            this->system_cfg = std::move(sc);

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // Return the result.
    return result;
}

// Specific function for reading C1.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readLaserCFG(
        const dpslr::common::ConsolidatedRecordStruct& record, float)
{
    // Variables.
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    // Tokens are: C1, DETAIL, CFG ID, TYPE, PRIM_WAVE, FREQ, ENERGY, PULSE_WIDTH, DIVERGENCE, PULSES_SEMITRAIN
    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Delete the current data.
    this->clearLaserConfiguration();

    // Check if size is correct.
    if (tokens.size() != 10)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != CfgLineString[static_cast<int>(ConfigurationLineEnum::LASER_CFG)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    else
        try
        {
            // New laser configuration struct
            LaserConfiguration lc;

            // Get the data.
            lc.detail = std::stoi(tokens[1]);
            lc.cfg_id =tokens[2];
            lc.type = tokens[3];
            lc.primary_wavelength = std::stod(tokens[4]);
            lc.fire_rate = std::stod(tokens[5]);
            lc.pulse_energy = std::stod(tokens[6]);
            lc.pulse_width = std::stod(tokens[7]);
            lc.beam_divergence = std::stod(tokens[8]);
            lc.pulses_outgoing_semitrain = std::stoi(tokens[9]);

            // Add the associated comments, the line number, and the tokens.
            lc.comment_block = record.comment_block;
            lc.line_number = record.line_number;
            lc.tokens = record.tokens;

            // Store the laser configuration struct
            this->laser_cfg = std::move(lc);

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // Return the result.
    return result;
}

// Specific function for reading C2.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readDetectorCFG(const dpslr::common::ConsolidatedRecordStruct& record, float v)
{
    // Variables.
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Delete the current data.
    this->clearDetectorConfiguration();

    // Check the data size for each version.
    // For v1.
    if (v >= 1 && v < 2 && tokens.size() != 14)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;

    // For v2.
    else if(v >= 2 && v < 3 && tokens.size() != 17)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;

    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != CfgLineString[static_cast<int>(ConfigurationLineEnum::DETECTOR_CFG)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;

    // All ok at this momment.
    else
        try
        {
            // New detector configuration struct
            DetectorConfiguration dc;

            // Common for v1.
            // Get the data.
            dc.detail = std::stoi(tokens[1]);
            dc.cfg_id = tokens[2];
            dc.type = tokens[3];
            dc.work_wavelength = std::stod(tokens[4]);
            dc.efficiency = std::stod(tokens[5]);
            dc.voltage = std::stod(tokens[6]);
            dc.dark_count = std::stod(tokens[7]);
            dc.out_pulse_type = tokens[8];
            dc.out_pulse_width = std::stod(tokens[9]);
            dc.spectral_filter = std::stod(tokens[10]);
            dc.transm_spectral_filter = std::stod(tokens[11]);
            dc.ext_signal_proc = tokens[13];

            // For v1
            if (v >= 1 && v < 2)
            {
                // TODO: check integrity. It should be positive
                dc.spatial_filter = ("-1" == tokens[12] ? decltype(dc.spatial_filter)() : std::stod(tokens[12]));
            }
            // For v2.
            else if(v >= 2 && v < 3 )
            {
                dc.spatial_filter = ("na" == tokens[12] ? decltype(dc.spatial_filter)() : std::stod(tokens[12]));

                // Get the amplifier related data.
                // If unknown or not used, all to undefined.
                // If is used, read each field.
                if(tokens[16] == "na" || !dpslr::helpers::BoolString(tokens[16]))
                {
                    dc.amp_gain = {};
                    dc.amp_bandwidth = {};
                    dc.used_amp = tokens[16] == "na" ? decltype(dc.used_amp)() : false;
                }
                else
                {
                    dc.amp_gain = (tokens[14] == "na" ? decltype(dc.amp_gain)() : std::stod(tokens[14]));
                    dc.amp_bandwidth = (tokens[15] == "na" ? decltype (dc.amp_bandwidth)() : std::stod(tokens[15]));
                    dc.used_amp = true;
                }
            }

            // Add the associated comments, the line number, and the tokens.
            dc.comment_block = record.comment_block;
            dc.line_number = record.line_number;
            dc.tokens = record.tokens;

            // Store the detector configuration struct
            this->detector_cfg = std::move(dc);

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // Return the result.
    return result;
}

// Specific function for reading C3.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readTimingCFG(
        const dpslr::common::ConsolidatedRecordStruct& record, float)
{
    // Variables.
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Delete the current data.
    this->clearTimingConfiguration();

    // Check if size is correct.
    if (tokens.size() != 8)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != CfgLineString[static_cast<int>(ConfigurationLineEnum::TIMING_CFG)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    
    // All ok at this momment.
    else
        try
        {
            // New format header struct.
            TimingConfiguration tc;

            // Get the data.
            tc.detail = std::stoi(tokens[1]);
            tc.cfg_id = tokens[2];
            tc.time_source = tokens[3];
            tc.frequency_source = tokens[4];
            tc.timer = tokens[5];
            tc.timer_serial = tokens[6];
            tc.epoch_delay = std::stod(tokens[7]);

            // Add the associated comments, the line number, and the tokens.
            tc.comment_block = record.comment_block;
            tc.line_number = record.line_number;
            tc.tokens = record.tokens;

            // Store the timing configuration struct
            this->timing_cfg = std::move(tc);

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // Return the result.
    return result;
}

// Specific function for reading C4.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readTransponderCFG(
        const dpslr::common::ConsolidatedRecordStruct&, float)
{
    // TODO
    return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
}

// Specific function for reading C5.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readSoftwareCFG(
        const dpslr::common::ConsolidatedRecordStruct& record, float v)
{
    // Variables.
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Delete the current data.
    this->clearSoftwareConfiguration();

    // Check if size is correct.
    if (tokens.size() != 7)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != CfgLineString[static_cast<int>(ConfigurationLineEnum::SOFTWARE_CFG)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // Check if version is at least 2.
    else if (v < 2.f)
        result = dpslr::common::RecordReadErrorEnum::VERSION_MISMATCH;

    // All ok at this momment.
    else
        try
        {
            // New format header struct.
            SoftwareConfiguration swc;

            // Get the data.
            swc.detail = std::stoi(tokens[1]);
            swc.cfg_id = tokens[2];
            swc.tracking_sw = tokens[3];
            swc.tracking_sw_ver = tokens[4];
            swc.processing_sw = tokens[5];
            swc.processing_sw_ver = tokens[6];

            // Add the associated comments, the line number, and the tokens.
            swc.comment_block = record.comment_block;
            swc.line_number = record.line_number;
            swc.tokens = record.tokens;

            // Store the timing configuration struct
            this->software_cfg = std::move(swc);

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // Return the result.
    return result;
}

// Specific function for reading C6.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readMeteoCFG(
        const dpslr::common::ConsolidatedRecordStruct& record, float version)
{
    // Variables.
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Delete the current data.
    this->clearMeteorologicalConfiguration();

    // Check if size is correct.
    if (tokens.size() != 12)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) !=
             CfgLineString[static_cast<int>(ConfigurationLineEnum::METEOROLOGICAL_CFG)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // Check if version is 2, since it does not exist in version 1
    else if (version < 2)
        result = dpslr::common::RecordReadErrorEnum::VERSION_MISMATCH;

    // All ok at this momment.
    else
        try
        {
            // New format header struct.
            MeteorologicalConfiguration mc;

            // Get the data.
            mc.detail = std::stoi(tokens[1]);
            mc.cfg_id = tokens[2];
            mc.press_manufacturer = tokens[3];
            mc.press_model = tokens[4];
            mc.press_sn = tokens[5];
            mc.temp_manufacturer = tokens[6];
            mc.temp_model = tokens[7];
            mc.temp_sn = tokens[8];
            mc.humid_manufacturer = tokens[9];
            mc.humid_model = tokens[10];
            mc.humid_sn = tokens[11];

            // Add the associated comments, the line number, and the tokens.
            mc.comment_block = record.comment_block;
            mc.line_number = record.line_number;
            mc.tokens = record.tokens;

            // Store the timing configuration struct
            this->meteorological_cfg = std::move(mc);

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // Return the result.
    return result;
}

// Specific function for reading C7.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readCalibrationCFG(
        const dpslr::common::ConsolidatedRecordStruct& record, float v)
{
    // Variables.
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Delete the current data.
    this->clearCalibrationTargetConfiguration();

    // Check if size is correct.
    if (tokens.size() != 10)
        result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) != CfgLineString[static_cast<int>(ConfigurationLineEnum::CALIBRATION_CFG)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // Check if version is at least 2.
    else if (v < 2.f)
        result = dpslr::common::RecordReadErrorEnum::VERSION_MISMATCH;

    // All ok at this momment.
    else
        try
        {
            // New format header struct.
            CalibrationTargetConfiguration ctc;

            // Get the data.
            ctc.detail = std::stoi(tokens[1]);
            ctc.cfg_id = tokens[2];
            ctc.target_name = tokens[3];
            ctc.distance = std::stod(tokens[4]);
            if (tokens[5] != "na")
                ctc.error = std::stod(tokens[5]);
            if (tokens[6] != "na")
                ctc.delays = std::stod(tokens[6]);
            if (tokens[7] != "na")
                ctc.energy = std::stod(tokens[7]);
            ctc.processing_sw = tokens[8];
            ctc.processing_sw_ver = tokens[9];

            // Add the associated comments, the line number, and the tokens.
            ctc.comment_block = record.comment_block;
            ctc.line_number = record.line_number;
            ctc.tokens = record.tokens;

            // Store the timing configuration struct
            this->calib_target_cfg = std::move(ctc);

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // Return the result.
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CONFIGURATION PRIVATE FUNCTIONS -----------------------------------------------------------------------------
// Generic function for reading single configuration line.
dpslr::common::RecordReadErrorEnum CRDConfiguration::readConfigurationLine(const dpslr::common::RecordLinePair &lpair,
                                                                       float version)
{
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;

    switch (static_cast<ConfigurationLineEnum>(lpair.first))
    {
        case ConfigurationLineEnum::SYSTEM_CFG:
        result = this->readSystemCFG(lpair.second, version);
        break;

        case ConfigurationLineEnum::LASER_CFG:
        result = this->readLaserCFG(lpair.second, version);
        break;

        case ConfigurationLineEnum::DETECTOR_CFG:
        result = this->readDetectorCFG(lpair.second, version);
        break;

        case ConfigurationLineEnum::TIMING_CFG:
        result = this->readTimingCFG(lpair.second, version);
        break;

        case ConfigurationLineEnum::TRANSPONDER_CFG:
        result = this->readTransponderCFG(lpair.second, version);
        break;

        case ConfigurationLineEnum::SOFTWARE_CFG:
        result = this->readSoftwareCFG(lpair.second, version);
        break;

        case ConfigurationLineEnum::METEOROLOGICAL_CFG:
        result = this->readMeteoCFG(lpair.second, version);
        break;

        case ConfigurationLineEnum::CALIBRATION_CFG:
        result = this->readCalibrationCFG(lpair.second, version);
        break;

    }

    return result;
}

// Helper function to generate A, B, C... of C0 line.
std::string CRDConfiguration::generateSystemCfgLine(float version) const
{
    // TODO check validity for mandatory lines
    if (!(this->system_cfg && this->laser_cfg && this->detector_cfg && this->timing_cfg))
        return "";

    // Base line
    std::stringstream line_c0;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_c0 << "C0"
                << ' ' << this->system_cfg->detail_type
                << ' ' << dpslr::helpers::numberToFixstr(this->system_cfg->transmit_wavelength, 10)
                << ' ' << this->system_cfg->system_cfg_id
                << ' ' << this->laser_cfg->cfg_id
                << ' ' << this->detector_cfg->cfg_id
                << ' ' << this->timing_cfg->cfg_id;

        if (transponder_cfg)
            line_c0 << ' ' << this->transponder_cfg->cfg_id;
    }

    // For version 2 only
    if (version >= 2 && version < 3)
    {
        // Non mandatory fields.
        if(this->software_cfg)
            line_c0 << ' ' << this->software_cfg->cfg_id;
        if(this->meteorological_cfg)
            line_c0 << ' ' << this->meteorological_cfg->cfg_id;
        if(this->calib_target_cfg)
            line_c0 << ' ' << this->calib_target_cfg->cfg_id;
    }

    // Return computed C0
    return line_c0.str();
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CONFIGURATION STRUCTS ---------------------------------------------------------------------------------------
std::string CRDConfiguration::LaserConfiguration::generateLine(float version) const
{
    // Base line
    std::stringstream line_c1;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_c1 << "C1"
                << ' ' << this->detail
                << ' ' << this->cfg_id
                << ' ' << this->type
                << ' ' << dpslr::helpers::numberToFixstr(this->primary_wavelength, 10)
                << ' ' << dpslr::helpers::numberToFixstr(this->fire_rate, 10)
                << ' ' << dpslr::helpers::numberToFixstr(this->pulse_energy, 10)
                << ' ' << dpslr::helpers::numberToFixstr(this->pulse_width, 6)
                << ' ' << dpslr::helpers::numberToFixstr(this->beam_divergence, 5)
                << ' ' << this->pulses_outgoing_semitrain;
    }

    // Return the C1
    return line_c1.str();
}

std::string CRDConfiguration::DetectorConfiguration::generateLine(float version) const
{
    // Base line
    std::stringstream line_c2;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_c2 << "C2"
                << ' ' << this->detail
                << ' ' << this->cfg_id
                << ' ' << this->type
                << ' ' << dpslr::helpers::numberToFixstr(this->work_wavelength,10)
                << ' ' << dpslr::helpers::numberToFixstr(this->efficiency, 6)
                << ' ' << dpslr::helpers::numberToFixstr(this->voltage, 5)
                << ' ' << dpslr::helpers::numberToFixstr(this->dark_count, 5)
                << ' ' << this->out_pulse_type
                << ' ' << dpslr::helpers::numberToFixstr(this->out_pulse_width, 5)
                << ' ' << dpslr::helpers::numberToFixstr(this->spectral_filter, 5)
                << ' ' << dpslr::helpers::numberToFixstr(this->transm_spectral_filter, 5);

        // Spatial filter is optional
        if (this->spatial_filter)
            line_c2 << ' ' << dpslr::helpers::numberToFixstr(this->spatial_filter.value(), 5);
        else
            line_c2 << ' ' << (version < 2 ? "-1" : "na");

        line_c2 << ' ' << this->ext_signal_proc;
    }

    // For version 2.
    if (version >= 2 && version < 3)
    {
        // If used amp is No (0) or not defined, gain and bandwidth are
        // set to na and amp in use to 0 (No) or na (unknown).
        if(!this->used_amp.value_or(false))
        {
            line_c2 << " na na " << (this->used_amp ? "0" : "na");
        }
        // Otherwise, we will print each vaule as separated.
        else
        {
            line_c2 << ' ' << (!this->amp_gain ? "na" : dpslr::helpers::numberToFixstr(this->amp_gain.value(), 6))
                    << ' ' << (!this->amp_bandwidth ? "na" : dpslr::helpers::numberToFixstr(this->amp_bandwidth.value(), 6))
                    << " 1";
        }
    }

    // Return the C2
    return line_c2.str();
}

std::string CRDConfiguration::TimingConfiguration::generateLine(float version) const
{
    // Base line
    std::stringstream line_c3;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_c3 << "C3"
                << ' ' << this->detail
                << ' ' << this->cfg_id
                << ' ' << this->time_source
                << ' ' << this->frequency_source
                << ' ' << this->timer
                << ' ' << this->timer_serial
                << ' ' << dpslr::helpers::numberToFixstr(this->epoch_delay, 6);
    }

    // Return the C3
    return line_c3.str();
}

std::string CRDConfiguration::TransponderConfiguration::generateLine(float) const
{
    // TODO
    // Base line
    std::stringstream line_c4;

    // Return the C4
    return line_c4.str();
}

std::string CRDConfiguration::SoftwareConfiguration::generateLine(float version) const
{
    // Base line
    std::stringstream line_c5;

    // For version 2.
    if (version >= 2 && version < 3)
    {
        line_c5 << "C5"
                << ' ' << this->detail
                << ' ' << this->cfg_id
                << ' ' << this->tracking_sw
                << ' ' << this->tracking_sw_ver
                << ' ' << this->processing_sw
                << ' ' << this->processing_sw_ver;
    }

    // Return the C5
    return line_c5.str();
}

std::string CRDConfiguration::MeteorologicalConfiguration::generateLine(float version) const
{
    // Base line
    std::stringstream line_c6;

    // Only version 2.
    if (version >= 2 && version < 3)
    {
        line_c6 << "C6"
                << ' ' << this->detail
                << ' ' << this->cfg_id
                << ' ' << this->press_manufacturer
                << ' ' << this->press_model
                << ' ' << this->press_sn
                << ' ' << this->temp_manufacturer
                << ' ' << this->temp_model
                << ' ' << this->temp_sn
                << ' ' << this->humid_manufacturer
                << ' ' << this->humid_model
                << ' ' << this->humid_sn;
    }

    // Return the C6
    return line_c6.str();
}

std::string CRDConfiguration::CalibrationTargetConfiguration::generateLine(float version) const
{
    // Base line
    std::stringstream line_c7;

    // For version 2.
    if (version >= 2 && version < 3)
    {
        line_c7 << "C7"
                << ' ' << this->detail
                << ' ' << this->cfg_id
                << ' ' << this->target_name
                << ' ' << dpslr::helpers::numberToStr(this->distance, 17, 5)
                << ' ' << (this->error ? dpslr::helpers::numberToStr(this->error.value(), 8, 2) : "na")
                << ' ' << (this->delays ? dpslr::helpers::numberToStr(this->delays.value(), 12, 8) : "na")
                << ' ' << (this->energy ? dpslr::helpers::numberToStr(this->energy.value(), 12, 2) : "na")
                << ' ' << this->processing_sw
                << ' ' << this->processing_sw_ver;
    }

    // Return the C7
    return line_c7.str();
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================


// ========== CRD DATA =================================================================================================

// --- CRD DATA CONST EXPRESSIONS --------------------------------------------------------------------------------------
const std::array<const char*, 10> CRDData::DataLineString {"10", "11", "12", "20", "21", "30", "40", "41", "50", "60"};
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD HEADER CLEAR FUNCTIONS --------------------------------------------------------------------------------------
void CRDData::clearAll()
{
    this->clearFullRateRecords();
    this->clearNormalPointRecords();
    this->clearMeteorologicalRecords();
    this->clearCalibrationRecords();
    this->clearRTCalibrationRecords();
    this->clearOverallCalibrationRecord();
    this->clearStatisticsRecord();
}

void CRDData::clearFullRateRecords() {this->fullrate_records.clear();}

void CRDData::clearNormalPointRecords() {this->normalpoint_records.clear();}

void CRDData::clearMeteorologicalRecords() {this->meteo_records.clear();}

void CRDData::clearCalibrationRecords() {this->cal_records.clear();}

void CRDData::clearRTCalibrationRecords() {this->rt_cal_records.clear();}

void CRDData::clearOverallCalibrationRecord() {this->cal_overall_record = {};}

void CRDData::clearStatisticsRecord() {this->stat_record = {};}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD DATA SETTERS ------------------------------------------------------------------------------------------------
void CRDData::addFullRateRecord(const CRDData::FullRateRecord &rec) {this->fullrate_records.push_back(rec);}

void CRDData::addNormalPointRecord(const CRDData::NormalPointRecord &rec) {this->normalpoint_records.push_back(rec);}

void CRDData::addMeteorologicalRecord(const CRDData::MeteorologicalRecord &rec) {this->meteo_records.push_back(rec);}

void CRDData::addRealTimeCalibrationRecord(const CRDData::CalibrationRecord &rec) {this->rt_cal_records.push_back(rec);}

void CRDData::addCalibrationRecord(const CRDData::CalibrationRecord &rec) {this->cal_records.push_back(rec);}

void CRDData::setOverallCalibrationRecord(const CalibrationRecord &rec) {this->cal_overall_record = rec;}

void CRDData::setStatisticsRecord(const StatisticsRecord &rec) {this->stat_record = rec;}

void CRDData::setFullRateRecords(const std::vector<CRDData::FullRateRecord> &rec) {this->fullrate_records = rec;}

void CRDData::setNormalPointRecords(const std::vector<CRDData::NormalPointRecord> &rec)
{this->normalpoint_records = rec;}

void CRDData::setMeteorologicalRecords(const std::vector<CRDData::MeteorologicalRecord> &rec)
{this->meteo_records = rec;}

void CRDData::setCalibrationRecords(const std::vector<CRDData::CalibrationRecord> &rec) {this->cal_records = rec;}

void CRDData::setRealTimeCalibrationRecords(const std::vector<CRDData::CalibrationRecord> &rec)
{this->rt_cal_records = rec;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD DATA GETTERS ------------------------------------------------------------------------------------------------
dpslr::common::FlightTimeData CRDData::fullRateFlightTimeData() const
{
    // Container
    dpslr::common::FlightTimeData data;

    // Get the X data (time tags).
    std::transform(this->fullrate_records.begin(), this->fullrate_records.end(), std::back_inserter(data),
        [](const CRDData::FullRateRecord& rec){return std::make_pair(rec.time_tag,rec.time_flight);});

    // Return the container.
    return data;
}

dpslr::common::FlightTimeData CRDData::normalPointFlightTimeData() const
{
    // Container
    dpslr::common::FlightTimeData data;

    // Get the X data (time tags).
    std::transform(this->normalpoint_records.begin(), this->normalpoint_records.end(), std::back_inserter(data),
        [](const CRDData::NormalPointRecord& rec){return std::make_pair(rec.time_tag,rec.time_flight);});

    // Return the container.
    return data;
}

const std::vector<CRDData::FullRateRecord> &CRDData::fullRateRecords() const {return this->fullrate_records;}

const std::vector<CRDData::NormalPointRecord> &CRDData::normalPointRecords() const {return this->normalpoint_records;}

const std::vector<CRDData::MeteorologicalRecord> &CRDData::meteorologicalRecords() const {return this->meteo_records;}

const std::vector<CRDData::CalibrationRecord> &CRDData::realTimeCalibrationRecord() const {return this->rt_cal_records;}

const std::vector<CRDData::CalibrationRecord> &CRDData::calibrationRecords() const {return this->cal_records;}

const dpslr::common::optional<CRDData::CalibrationRecord> &CRDData::calibrationOverallRecord() const
{return this->cal_overall_record;}

const dpslr::common::optional<CRDData::StatisticsRecord> &CRDData::statisticsRecord() const {return this->stat_record;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD NON-CONST DATA GETTERS ------------------------------------------------------------------------------------------------
std::vector<CRDData::FullRateRecord> &CRDData::fullRateRecords() {return this->fullrate_records;}

std::vector<CRDData::NormalPointRecord> &CRDData::normalPointRecords() {return this->normalpoint_records;}

std::vector<CRDData::MeteorologicalRecord> &CRDData::meteorologicalRecords() {return this->meteo_records;}

std::vector<CRDData::CalibrationRecord> &CRDData::realTimeCalibrationRecord() {return this->rt_cal_records;}

std::vector<CRDData::CalibrationRecord> &CRDData::calibrationRecords() {return this->cal_records;}

dpslr::common::optional<CRDData::CalibrationRecord> &CRDData::calibrationOverallRecord()
{return this->cal_overall_record;}

dpslr::common::optional<CRDData::StatisticsRecord> &CRDData::statisticsRecord() {return this->stat_record;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD GENERATE DATA LINES -----------------------------------------------------------------------------------------
std::string CRDData::generateDataLines(float version, DataGenerationOptionEnum option) const
{
    // TODO: if there are for example no meteo lines, endl is inserted. This fails in several points.
    // Full rate lines
    std::stringstream data;

    // Lines order: 20, 41, 40 real time, 40 overall, 50, 10 / 11
    data << this->generateMeteoLines(version) << std::endl;
    data << this->generateCalibrationLines(version) << std::endl;
    if (this->stat_record)
        data << this->generateStatisticsLine(version) << std::endl;

    // Generate the lines 10 / 11 depending on the option.
    if(option == DataGenerationOptionEnum::FULL_RATE || option == DataGenerationOptionEnum::BOTH_DATA)
        data << this->generateFullRateLines(version);
    if(option == DataGenerationOptionEnum::NORMAL_POINT || option == DataGenerationOptionEnum::BOTH_DATA)
        data << this->generateNormalPointLines(version);

    // Return all the lines.
    return data.str();
}

std::string CRDData::generateFullRateLines(float version) const
{
    // Full rate lines
    std::stringstream fr_line;

    // Generate the FR line for each record.
    for(const auto& fr : this->fullrate_records)
    {
        // Add the comment block.
        if(!fr.comment_block.empty())
            fr_line << fr.generateCommentBlock() << std::endl;
        fr_line << fr.generateLine(version) << std::endl;
    }

    // Return all the lines.
    return fr_line.str().substr(0, fr_line.str().find_last_of('\n'));
}

std::string CRDData::generateNormalPointLines(float version) const
{
    // Normal Points lines
    std::stringstream np_line;

    // Generate the NP line for each record.
    for(const auto& np : this->normalpoint_records)
    {
        // Add the comment block.
        if(!np.comment_block.empty())
            np_line << np.generateCommentBlock() << std::endl;
        np_line << np.generateLine(version) << std::endl;
    }

    // Return all the lines.
    return np_line.str().substr(0, np_line.str().find_last_of('\n'));

}

std::string CRDData::generateMeteoLines(float version) const
{
    // Meteorological lines
    std::stringstream meteo_line;

    // Generate the meteo line for each record.
    for(const auto& meteo : this->meteo_records)
    {
        // Add the comment block.
        if(!meteo.comment_block.empty())
            meteo_line << meteo.generateCommentBlock() << std::endl;
        meteo_line << meteo.generateLine(version) << std::endl;
    }

    // Return all the lines.
    return meteo_line.str().substr(0, meteo_line.str().find_last_of('\n'));
}

std::string CRDData::generateCalibrationLines(float version) const
{
    // Calibration lines
    std::stringstream cal_line;

    // Generate the detail calibration line for each record. Lines 41 for v2 or 40 for v1.
    for(const auto& cal : this->cal_records)
    {
        // Add the comment block.
        if(!cal.comment_block.empty())
            cal_line << cal.generateCommentBlock() << std::endl;
        cal_line << cal.generateLine(version) << std::endl;
    }

    // Generate the real time calibration line for each record. Lines 40 for v2.
    if(version >= 2 && version < 3)
        for(const auto& cal : this->rt_cal_records)
        {
            // Add the comment block.
            if(!cal.comment_block.empty())
                cal_line << cal.generateCommentBlock() << std::endl;
            cal_line << cal.generateLine(version) << std::endl;
        }

    // Generate the overall calibration line for v2.
    if(version >= 2 && version < 3 && this->cal_overall_record)
    {
        // Add the comment block.
        if(!this->cal_overall_record->comment_block.empty())
            cal_line << this->cal_overall_record->generateCommentBlock() << std::endl;
        cal_line << this->cal_overall_record->generateLine(version) << std::endl;
    }

    // Return all the lines.
    return cal_line.str().substr(0, cal_line.str().find_last_of('\n'));
}

std::string CRDData::generateStatisticsLine(float version) const
{
    // Statistic line
    std::stringstream stat_line;

    // Generate the statistic line.
    if(this->stat_record)
    {
        // Add the comment block.
        if(!this->stat_record->comment_block.empty())
            stat_line << this->stat_record->generateCommentBlock() << std::endl;
        stat_line << this->stat_record->generateLine(version);
    }

    // Return the line
    return stat_line.str();
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD READ DATA LINES ---------------------------------------------------------------------------------------------
dpslr::common::RecordReadErrorEnum CRDData::readDataLine(const dpslr::common::RecordLinePair &lpair, float version)
{
    switch (static_cast<DataRecordTypeEnum>(lpair.first))
    {
        case DataRecordTypeEnum::FULL_RATE_RECORD:
        return this->readFRDataLine(lpair.second, version);
        break;

        case DataRecordTypeEnum::NORMAL_POINT_RECORD:
        return this->readNPDataLine(lpair.second, version);
        break;

        case DataRecordTypeEnum::RANGE_SUPPLEMENT_RECORD:
        return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
        break;

        case DataRecordTypeEnum::METEO_RECORD:
        return this->readMeteoDataLine(lpair.second, version);
        break;

        case DataRecordTypeEnum::METEO_SUP_RECORD:
        return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
        break;

        case DataRecordTypeEnum::POINTING_ANGLES_RECORD:
        return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
        break;

        case DataRecordTypeEnum::CALIBRATION_OVERALL_RECORD:
        case DataRecordTypeEnum::CALIBRATION_DETAIL_RECORD:
        return this->readCalDataLine(lpair.second, version);
        break;

        case DataRecordTypeEnum::STATISTICS_RECORD:
        return this->readStatisticsData(lpair.second, version);
        break;

        case DataRecordTypeEnum::COMPATIBILITY_RECORD:
        return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
        break;

        default:
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    }
}

dpslr::common::RecordReadErrorMultimap CRDData::readData(const dpslr::common::RecordLinesVector &rec_v, float version)
{
    // Aux variables.
    dpslr::common::RecordLinePair rec_pair;
    dpslr::common::RecordReadErrorMultimap error_map;
    dpslr::common::RecordReadErrorEnum error;
    int pos;

    // First, clear the data.
    this->clearAll();

    // Read the line and store the error, if any.
    for (const auto& rec : rec_v)
    {
        // Check that the record is a header record.
        if(dpslr::helpers::find(DataLineString, rec.getIdToken(), pos))
        {
            // Store the record type in a pair.
            rec_pair = {pos, rec};

            // Read the record to store the data in memory.
            error = this->readDataLine(rec_pair, version);

            // Check for errors.
            if(error != dpslr::common::RecordReadErrorEnum::NOT_ERROR)
                error_map.emplace(static_cast<int>(error), rec);
        }
        else
        {
            error_map.emplace(static_cast<int>(dpslr::common::RecordReadErrorEnum::BAD_TYPE), rec);
        }
    }

    // Return the map with the errors. If no errors, the map will be empty.
    return error_map;
}

dpslr::common::RecordReadErrorEnum CRDData::readFRDataLine(const dpslr::common::ConsolidatedRecordStruct &record, float v)
{
    // Struct.
    FullRateRecord fr_record;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check the data size for each version.
    // For v1.
    if (v >= 1 && v < 2 && tokens.size() != 9)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // For v2.
    else if(v >= 2 && v < 3 && tokens.size() != 10)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (tokens[0] != DataLineString[static_cast<int>(DataRecordTypeEnum::FULL_RATE_RECORD)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    
    // All ok at this momment.
    else
        try
        {
            // Get the data.
            fr_record.time_tag = std::stold(tokens[1]);
            fr_record.time_flight = std::stold(tokens[2]);
            fr_record.system_cfg_id = tokens[3];
            fr_record.epoch_event = static_cast<EpochEventEnum>(std::stoi(tokens[4]));
            fr_record.filter_flag = static_cast<FilterFlagEnum>(std::stoi(tokens[5]));
            fr_record.detector_channel = std::stoi(tokens[6]);
            fr_record.stop_number = std::stoi(tokens[7]);
            fr_record.receive_amp = (tokens[8] == "na" || std::stoi(tokens[8]) == 0) ?
                        decltype(fr_record.receive_amp)() : std::stoi(tokens[8]);

            // Only for v2.
            if(v >= 2 && v < 3)
                fr_record.transmit_amp = (tokens[9] == "na" || std::stoi(tokens[9]) == 0) ?
                            decltype(fr_record.transmit_amp)() : std::stoi(tokens[9]);

            // Add the associated comments, the line number, and the tokens.
            fr_record.comment_block = record.comment_block;
            fr_record.line_number = record.line_number;
            fr_record.tokens = record.tokens;

        } catch (...)
        {
            return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // All ok, insert the struct.
    this->fullrate_records.push_back(fr_record);

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

dpslr::common::RecordReadErrorEnum CRDData::readNPDataLine(const dpslr::common::ConsolidatedRecordStruct &record, float v)
{
    // Struct.
    NormalPointRecord np_record;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check the data size for each version.
    // For v1.
    if (v >= 1 && v < 2 && tokens.size() != 13)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // For v2.
    else if(v >= 2 && v < 3 && tokens.size() != 14)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (tokens[0] != DataLineString[static_cast<int>(DataRecordTypeEnum::NORMAL_POINT_RECORD)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    
    // All ok at this momment.
    else
        try
        {
            // Get the data.
            np_record.time_tag = std::stold(tokens[1]);
            np_record.time_flight = std::stold(tokens[2]);
            np_record.system_cfg_id = tokens[3];
            np_record.epoch_event = static_cast<EpochEventEnum>(std::stoi(tokens[4]));
            np_record.window_length = std::stod(tokens[5]);
            np_record.raw_ranges = std::stoi(tokens[6]);

            // RMS, skew and kurtosis (optional values).
            if(v >= 1 && v < 2)
            {
                np_record.bin_rms = tokens[7] == "-1" ? decltype(np_record.bin_rms)() : std::stod(tokens[7]);
                np_record.bin_skew = tokens[8] == "-1" ? decltype(np_record.bin_skew)() : std::stod(tokens[8]);
                np_record.bin_kurtosis = tokens[9] == "-1" ? decltype(np_record.bin_kurtosis)() : std::stod(tokens[9]);
                np_record.bin_peak = tokens[10] == "-1" ? decltype(np_record.bin_peak)() : std::stod(tokens[10]);

            }
            else if(v>=2 && v<3)
            {
                np_record.bin_rms = tokens[7] == "na" ? decltype(np_record.bin_rms)() : std::stod(tokens[7]);
                np_record.bin_skew = tokens[8] == "na" ? decltype(np_record.bin_skew)() : std::stod(tokens[8]);
                np_record.bin_kurtosis = tokens[9] == "na" ? decltype(np_record.bin_kurtosis)() : std::stod(tokens[9]);
                np_record.bin_peak = tokens[10] == "na" ? decltype(np_record.bin_peak)() : std::stod(tokens[10]);
            }

            // Rest of the data.
            np_record.return_rate = std::stod(tokens[11]);
            np_record.detector_channel = std::stoi(tokens[12]);

            // For version 2 only.
            if(v>=2 && v<3)
            {
                np_record.snr = (tokens[13] == "na") ? decltype(np_record.snr)() : std::stod(tokens[13]);
            }

            // Add the associated comments, the line number, and the tokens.
            np_record.comment_block = record.comment_block;
            np_record.line_number = record.line_number;
            np_record.tokens = record.tokens;

        } catch (...)
        {
            return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // All ok, insert the struct.
    this->normalpoint_records.push_back(np_record);

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

dpslr::common::RecordReadErrorEnum CRDData::readMeteoDataLine(const dpslr::common::ConsolidatedRecordStruct &record, float v)
{
    // Struct.
    MeteorologicalRecord meteo_record;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check the data size for each version.
    // For v1.
    if (v >= 1 && v < 3 && tokens.size() != 6)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (tokens[0] != DataLineString[static_cast<int>(DataRecordTypeEnum::METEO_RECORD)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    
    // All ok at this momment.
    else
        try
        {
            // Get the data.
            meteo_record.time_tag = std::stold(tokens[1]);
            meteo_record.surface_pressure = std::stod(tokens[2]);
            meteo_record.surface_temperature =  std::stod(tokens[3]);
            meteo_record.surface_relative_humidity = std::stod(tokens[4]);
            meteo_record.values_origin = static_cast<MeteoOriginEnum>(std::stoi(tokens[5]));

            // Add the associated comments, the line number, and the tokens.
            meteo_record.comment_block = record.comment_block;
            meteo_record.line_number = record.line_number;
            meteo_record.tokens = record.tokens;

        } catch (...)
        {
            return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // All ok, insert the struct.
    this->meteo_records.push_back(meteo_record);

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

dpslr::common::RecordReadErrorEnum CRDData::readCalDataLine(const dpslr::common::ConsolidatedRecordStruct &record, float v)
{
    // Struct.
    CalibrationRecord cal_record;

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Flags for check the record type.
    bool is_detail = tokens[0] == DataLineString[static_cast<int>(DataRecordTypeEnum::CALIBRATION_DETAIL_RECORD)];
    bool is_overal = tokens[0] == DataLineString[static_cast<int>(DataRecordTypeEnum::CALIBRATION_OVERALL_RECORD)];

    // Check the data size for each version.
    if (v >= 1 && v < 2 && tokens.size() != 16)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    else if (v >= 2 && v < 3 && tokens.size() != 18)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;

    // Check the version mismatch
    if (v >= 1 && v < 2 && is_detail)
        return dpslr::common::RecordReadErrorEnum::VERSION_MISMATCH;
    // Check the record type.
    else if (!is_detail && !is_overal)
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;

    // All ok at this momment.
    try
    {
        // Get the data.
        cal_record.time_tag = std::stold(tokens[1]);
        cal_record.data_type = static_cast<DataTypeEnum>(std::stoi(tokens[2]));
        cal_record.system_cfg_id = tokens[3];

        // Optional data.
        cal_record.data_recorded = (tokens[4] == "na" || std::stoi(tokens[4]) == -1) ?
                    dpslr::common::optional<int>() : std::stoi(tokens[4]);
        cal_record.data_used = (tokens[5] == "na" || std::stoi(tokens[5]) == -1) ?
                    dpslr::common::optional<int>() : std::stoi(tokens[5]);
        cal_record.target_dist_1w = (tokens[6] == "na" || tokens[6] == "-1") ?
                    dpslr::common::optional<double>() : std::stod(tokens[6]);

        // Rest of the data.
        cal_record.calibration_delay = std::stod(tokens[7]);
        cal_record.delay_shift = std::stod(tokens[8]);
        cal_record.rms = std::stod(tokens[9]);
        cal_record.skew = std::stod(tokens[10]);
        cal_record.kurtosis = std::stod(tokens[11]);
        cal_record.peak = std::stod(tokens[12]);
        cal_record.cal_type = static_cast<CalibrationTypeEnum>(std::stoi(tokens[13]));
        cal_record.shift_type = static_cast<ShiftTypeEnum>(std::stoi(tokens[14]));
        cal_record.detector_channel = std::stoi(tokens[15]);

        // Add the associated comments, the line number, and the tokens.
        cal_record.comment_block = record.comment_block;
        cal_record.line_number = record.line_number;
        cal_record.tokens = record.tokens;

        // For v2 only.
        if (v >= 2 && v < 3)
        {
            cal_record.span = static_cast<CalibrationSpan>(std::stoi(tokens[16]));
            cal_record.return_rate = (tokens[17] == "na") ? decltype (cal_record.return_rate)() : std::stod(tokens[17]);
        }
    } catch (...)
    {
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Check the type of calibration for storing it.
    if (v >= 1 && v < 2)
    {
        cal_record.is_overall = false;
        this->cal_records.push_back(cal_record);
    }
    else if(v >= 2 && v < 3)
    {
        // Check span.
        if(cal_record.span == CalibrationSpan::NOT_APPLICABLE)
            return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
        else if(is_overal && cal_record.span == CalibrationSpan::REAL_TIME)
        {
            cal_record.is_overall = false;
            this->rt_cal_records.push_back(cal_record);
        }
        else if(is_overal && cal_record.span != CalibrationSpan::REAL_TIME)
        {
            // Set the cal record overall as true.
            cal_record.is_overall = true;

            // Create the new calibration overall.
            this->cal_overall_record = cal_record;
        }
        else if(is_detail && (cal_record.span == CalibrationSpan::COMBINED
                              || cal_record.span == CalibrationSpan::REAL_TIME))
            return dpslr::common::RecordReadErrorEnum::OTHER_ERROR;
        else if(is_detail)
        {
            cal_record.is_overall = false;
            this->cal_records.push_back(cal_record);
        }
        else
            return dpslr::common::RecordReadErrorEnum::OTHER_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

dpslr::common::RecordReadErrorEnum CRDData::readStatisticsData(const dpslr::common::ConsolidatedRecordStruct &record, float)
{
    // Delete the current data.
    this->clearStatisticsRecord();

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check the data size.
    if (tokens.size() != 7)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (tokens[0] != DataLineString[static_cast<int>(DataRecordTypeEnum::STATISTICS_RECORD)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;

    // All ok at this momment.
    try
    {
        // Create statistic struct.
        StatisticsRecord sr;

        // Get the data.
        sr.system_cfg_id = tokens[1];
        sr.rms = std::stod(tokens[2]);
        sr.skew = (tokens[3] == "na" || tokens[3] == "-1") ? decltype(sr.skew)() : std::stod(tokens[3]);
        sr.kurtosis = (tokens[4] == "na" || tokens[4] == "-1") ?
                    decltype(sr.kurtosis)() : std::stod(tokens[4]);
        sr.peak = (tokens[5] == "na" || tokens[5] == "-1") ? decltype(sr.peak)() : std::stod(tokens[5]);
        sr.quality = static_cast<DataQualityEnum>(std::stoi(tokens[6]));

        // Add the associated comments, the line number, and the tokens.
        sr.comment_block = record.comment_block;
        sr.line_number = record.line_number;
        sr.tokens = record.tokens;

        // Store stats record
        this->stat_record = std::move(sr);

    } catch (...)
    {
        // Return the error.
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD DATA STRUCTS ------------------------------------------------------------------------------------------------
std::string CRDData::FullRateRecord::generateLine(float version) const
{
    // Base line.
    std::stringstream line_10;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_10 << "10"
                << ' ' << dpslr::helpers::numberToStr(this->time_tag, 18, 12)
                << ' ' << dpslr::helpers::numberToStr(this->time_flight, 18, 12)
                << ' ' << this->system_cfg_id
                << ' ' << static_cast<int>(this->epoch_event)
                << ' ' << static_cast<int>(this->filter_flag)
                << ' ' << this->detector_channel
                << ' ' << this->stop_number;

        // Check if we have receive amp.
        if(!this->receive_amp)
            line_10 << ' ' << (version >= 2 ? "na" : std::to_string(0));
        else
            line_10 << ' ' << this->receive_amp.value();
    }

    // For version 2 only.
    if (version >= 2 && version < 3)
    {
        line_10 << ' ' << (!this->transmit_amp ? "na" : std::to_string(this->transmit_amp.value()));
    }

    // Return LINE 10.
    return line_10.str();
}

std::string CRDData::NormalPointRecord::generateLine(float version) const
{
    // Base line.
    std::stringstream line_11;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_11 << "11"
                << ' ' << dpslr::helpers::numberToStr(this->time_tag, 18, 12)
                << ' ' << dpslr::helpers::numberToStr(this->time_flight, 18, 12)
                << ' ' << this->system_cfg_id
                << ' ' << static_cast<int>(this->epoch_event)
                << ' ' << dpslr::helpers::numberToStr(this->window_length, 6, 1)
                << ' ' << this->raw_ranges;

        // Optional values.
        if(version>= 1 && version < 2)
        {
            line_11 << ' ' << (!this->bin_rms ? "-1" : dpslr::helpers::numberToStr(this->bin_rms.value(), 9, 1))
                    << ' ' << (!this->bin_skew ? "-1" : dpslr::helpers::numberToStr(this->bin_skew.value(), 7, 3))
                    << ' ' << (!this->bin_kurtosis ? "-1" : dpslr::helpers::numberToStr(this->bin_kurtosis.value(), 7, 3))
                    << ' ' << (!this->bin_peak ? "-1" : dpslr::helpers::numberToStr(this->bin_peak.value(), 9, 1));
        }
        else if (version >= 2 && version < 3)
        {
            line_11 << ' ' << (this->bin_rms ? dpslr::helpers::numberToStr(this->bin_rms.value(), 9, 1) : "na")
                    << ' ' << (this->bin_skew ? dpslr::helpers::numberToStr(this->bin_skew.value(), 7, 3) : "na")
                    << ' ' << (this->bin_kurtosis ? dpslr::helpers::numberToStr(this->bin_kurtosis.value(), 7, 3) : "na")
                    << ' ' << (this->bin_peak ? dpslr::helpers::numberToStr(this->bin_peak.value(), 9, 1) : "na");
        }

        // Rest of the data.
        line_11 << ' ' << dpslr::helpers::numberToStr(this->return_rate, 5, 1)
                << ' ' << this->detector_channel;
    }

    // For version 2 only.
    if (version >= 2 && version < 3)
    {
        line_11 << ' ' << (this->snr ? dpslr::helpers::numberToStr(this->snr.value(), 5, 1) : "na");
    }

    // Return LINE 11.
    return line_11.str();
}

std::string CRDData::MeteorologicalRecord::generateLine(float version) const
{
    // Base line.
    std::stringstream line_20;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_20 << "20"
                << ' ' << dpslr::helpers::numberToStr(this->time_tag, 18, 12)
                << ' ' << dpslr::helpers::numberToStr(this->surface_pressure, 7, 2)
                << ' ' << dpslr::helpers::numberToStr(this->surface_temperature, 6, 2)
                << ' ' << dpslr::helpers::numberToStr(this->surface_relative_humidity, 4, 1)
                << ' ' << static_cast<int>(this->values_origin);
    }

    // Return LINE 20.
    return line_20.str();
}

std::string CRDData::CalibrationRecord::generateLine(float version) const
{
    // Base line.
    std::stringstream line;

    // First, we need to detect if the struct is a line 40 or line 41.
    // For version 1, we only have lines 40.
    if (version >= 1 && version < 2)
        line << "40";

    // For v2, if we have the flag span as real time or combined, then is line 40. Also we need the flag is_overall
    // to true. Remember call check data integrity before call this function to ckeck integrity.
    else if(version >= 2 && version < 3)
    {
        line << ((this->span == CRDData::CalibrationSpan::REAL_TIME || this->is_overall) ? "40" : "41");
    }

    // For v1 and v2.
    if (version >= 1 && version < 3)
    {
        line << ' ' << dpslr::helpers::numberToStr(this->time_tag, 18, 12)
             << ' ' << static_cast<int>(this->data_type)
             << ' ' << this->system_cfg_id;

        // Optional values.
        if(version>= 1 && version < 2)
        {
            line << ' ' << this->data_recorded.value_or(-1)
                 << ' ' << this->data_used.value_or(-1)
                 << ' ' << (!this->target_dist_1w ? "-1" : dpslr::helpers::numberToStr(this->target_dist_1w.value(), 7, 3));
        }
        else if(version >= 2 && version < 3)
        {
            line << ' ' << (!this->data_recorded ? "na" : std::to_string(this->data_recorded.value()))
                 << ' ' << (!this->data_used ? "na" : std::to_string(this->data_used.value()))
                 << ' ' << (!this->target_dist_1w ? "na" : dpslr::helpers::numberToStr(this->target_dist_1w.value(), 7, 3));
        }

        // Rest of the data.
        line << ' ' << dpslr::helpers::numberToStr(this->calibration_delay, 10, 1)
             << ' ' << dpslr::helpers::numberToStr(this->delay_shift, 8, 1)
             << ' ' << dpslr::helpers::numberToStr(this->rms, 6, 1)
             << ' ' << dpslr::helpers::numberToStr(this->skew, 7, 3)
             << ' ' << dpslr::helpers::numberToStr(this->kurtosis, 7, 3)
             << ' ' << dpslr::helpers::numberToStr(this->peak, 6, 1)
             << ' ' << static_cast<int>(this->cal_type)
             << ' ' << static_cast<int>(this->shift_type)
             << ' ' << this->detector_channel;
    }

    // For version 2 only.
    if (version >= 2 && version < 3)
        line << ' ' << static_cast<int>(this->span)
             << ' ' << (!this->return_rate ? "na" : dpslr::helpers::numberToStr(this->return_rate.value(), 5, 1));

    // Return line 40 - 41.
    return line.str();
}

std::string CRDData::StatisticsRecord::generateLine(float version) const
{
    // Base line.
    std::stringstream line_50;

    // For version 1 and 2.
    if (version >= 1 && version < 3)
    {
        line_50 << "50"
                << ' ' << this->system_cfg_id
                << ' ' << dpslr::helpers::numberToStr(this->rms, 6, 1);

        // Optional data.
        if(version >= 1 && version < 2)
        {
            line_50 << ' ' << dpslr::helpers::numberToStr(this->skew.value_or(-1), 7, 3)
                    << ' ' << dpslr::helpers::numberToStr(this->kurtosis.value_or(-1), 7, 3)
                    << ' ' << dpslr::helpers::numberToStr(this->peak.value_or(-1), 6, 1);
        }
        else if(version >= 2 && version < 3)
        {
            line_50 << ' ' << (!this->skew ? "na" : dpslr::helpers::numberToStr(this->skew.value(), 7, 3))
                    << ' ' << (!this->kurtosis ? "na" : dpslr::helpers::numberToStr(this->kurtosis.value(), 7, 3))
                    << ' ' << (!this->peak ? "na" : dpslr::helpers::numberToStr(this->peak.value(), 6, 1));
        }

        // Rest of the data.
        line_50 << ' ' << static_cast<int>(this->quality);
    }

    // Return LINE 50.
    return line_50.str();
}
// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================

// ========== CRD ======================================================================================================

// --- CRD CONST EXPRESSIONS -------------------------------------------------------------------------------------------
const std::array<const char*, 6> CRD::ExtensionsString {"frd", "npt", "fr2", "np2", "qlk", "crd"};
const std::array<const char*, 2> CRD::EndRecordsString {"H8", "H9"};
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CONSTRUCTORS ------------------------------------------------------------------------------------------------
CRD::CRD(float version):
    empty_(false)
{
    // Set the version and creation time at Format Header
    this->header.formatHeader()->crd_version = version;
    this->header.formatHeader()->crd_production_date = dpslr::common::HRTimePoint::clock::now();
}

CRD::CRD(const std::string &crd_filepath, OpenOptionEnum option)
{
    this->openCRDFile(crd_filepath, option);
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD CLEAR METHODS -----------------------------------------------------------------------------------------------
void CRD::clearCRD()
{
    // Clear the contents.
    this->clearCRDContents();

    // Clear the error storage.
    this->last_read_error_ = ReadFileErrorEnum::NOT_ERROR;
    this->last_error_record_ = {};
    this->read_header_errors.clear();
    this->read_cfg_errors.clear();
    this->read_data_errors.clear();

    // Clear others.
    this->crd_filename.clear();
    this->crd_fullpath.clear();

}

void CRD::clearCRDContents()
{
    // Clear the data, configuration and header and se to empty.
    this->header.clearAll();
    this->configuration.clearAll();
    this->data.clearAll();

    this->empty_ = true;
}

void CRD::clearCRDData()
{
    this->data.clearAll();
}

void CRD::clearCRDHeader()
{
   this->header.clearAll();
}

void CRD::clearCRDConfiguration()
{
    this->configuration.clearAll();
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD GETTERS -----------------------------------------------------------------------------------------------------
CRDHeader &CRD::getHeader() {return this->header;}

CRDConfiguration &CRD::getConfiguration() {return this->configuration;}

CRDData &CRD::getData() {return this->data;}

const CRDHeader &CRD::getHeader() const {return this->header;}

const CRDConfiguration &CRD::getConfiguration() const {return this->configuration;}

const CRDData &CRD::getData() const {return this->data;}

const dpslr::common::RecordReadErrorMultimap &CRD::getReadHeaderErrors() const {return this->read_header_errors;}

const dpslr::common::RecordReadErrorMultimap &CRD::getReadCfgErrors() const {return this->read_cfg_errors;}

const dpslr::common::RecordReadErrorMultimap &CRD::getReadDataErrors() const {return this->read_data_errors;}

CRD::ReadFileErrorEnum CRD::getLastReadError() const {return this->last_read_error_;}

const dpslr::common::optional<dpslr::common::ConsolidatedRecordStruct> &CRD::getLastReadErrorRecord() const
{return this->last_error_record_;}

const std::string &CRD::getSourceFilename() const {return this->crd_filename;}

const std::string &CRD::getSourceFilepath() const {return this->crd_fullpath;}

std::string CRD::getStandardFilename(TargetIdOptionEnum option) const
{
    // Variables.
    std::string filename;

    // Check the preconditions.
    if(!this->header.formatHeader() || !this->header.sessionHeader() ||
            !this->header.stationHeader() || !this->header.targetHeader())
        return "";

    // For v2 only.
    if(this->header.formatHeader()->crd_version >= 2 && this->header.formatHeader()->crd_version < 3)
    {
        // For non ILRS tracking.
        if(!this->header.stationHeader()->network.empty() && this->header.stationHeader()->network != "ILRS")
            filename.append(dpslr::helpers::toLower(this->header.stationHeader()->network) + '_');
    }

    // Get the CDP pad identifier for station
    filename.append(std::to_string(this->header.stationHeader()->cdp_pad_identifier) + '_');

    // Get the target identifier.
    switch (option)
    {
        case TargetIdOptionEnum::ILRS_ID:
        filename.append(this->header.targetHeader()->ilrsid + "_crd_");
        break;

        case TargetIdOptionEnum::SHORT_COSPAR:
        filename.append(dpslr::utils::ilrsidToShortcospar(this->header.targetHeader()->ilrsid) + "_crd_");
        break;

        case TargetIdOptionEnum::COSPAR:
        filename.append(dpslr::utils::ilrsidToCospar(this->header.targetHeader()->ilrsid) + "_crd_");
        break;

        case TargetIdOptionEnum::NORAD:
        filename.append(this->header.targetHeader()->norad + "_crd_");
        break;

        case TargetIdOptionEnum::TARGET_NAME:
        filename.append(dpslr::helpers::toLower(this->header.targetHeader()->name) + "_crd_");
        break;
    }

    // Append the starting date of the pass from H4.
    std::time_t time = std::chrono::system_clock::to_time_t(this->header.sessionHeader()->start_time);
    std::tm* tm = gmtime(&time);
    char start_date[14];
    std::strftime(start_date, 14, "%Y%m%d_%H%M", tm);
    filename.append(start_date);

    // Append the data release.
    std::stringstream rr;
    rr << std::setw(2) << std::setfill('0') << this->header.sessionHeader()->data_release;
    filename.append('_' + rr.str() + '.');

    // Get the data type.
    if(this->header.formatHeader()->crd_version >= 1 && this->header.formatHeader()->crd_version < 2)
        filename.append(CRD::ExtensionsString[static_cast<int>(this->header.sessionHeader()->data_type)]);
    else if(this->header.formatHeader()->crd_version >= 2 && this->header.formatHeader()->crd_version < 3)
        filename.append(CRD::ExtensionsString[static_cast<int>(this->header.sessionHeader()->data_type)+2]);

    // Return the convention filename.
    return filename;
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD OBSERVERS ---------------------------------------------------------------------------------------------------
bool CRD::empty() const {return this->empty_;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CRD FILE METHODS ------------------------------------------------------------------------------------------------
CRD::ReadFileErrorEnum CRD::openCRDFile(const std::string &crd_filepath, CRD::OpenOptionEnum open_option)
{
    // Variables.
    ReadFileErrorEnum error = ReadFileErrorEnum::NOT_ERROR;
    dpslr::common::RecordLinesVector data_vector;
    dpslr::common::RecordLinesVector cfg_vector;
    dpslr::common::RecordLinesVector header_vector;
    float version = 1.;
    bool header_finished = false;
    bool cfg_finished = false;
    bool data_finished = false;
    bool eos_finished = false;
    bool eof_finished = false;
    bool read_finished = false;

    // Clear the CRD.
    this->clearCRD();

    // Open the file using our custom input file stream.
    dpslr::helpers::InputFileStream crd_stream(crd_filepath);

    // Check if the stream is open.
    if(!crd_stream.is_open())
    {
        this->last_read_error_ = ReadFileErrorEnum::FILE_NOT_FOUND;
        return ReadFileErrorEnum::FILE_NOT_FOUND;
    }

    // Check if the stream is empty.
    if(crd_stream.isEmpty())
    {
        this->last_read_error_ = ReadFileErrorEnum::FILE_EMPTY;
        return ReadFileErrorEnum::FILE_EMPTY;
    }

    // Store the file path and name.
    this->crd_fullpath = crd_filepath;
    this->crd_filename = dpslr::helpers::split<std::vector<std::string>>(crd_filepath, "/").back();

    // Open the header.
    while (!read_finished)
    {
        // Auxiliar variables.
        dpslr::common::ConsolidatedRecordStruct record;
        CRD::ReadRecordResultEnum read_result;

        // Get the next record.
        read_result = this->readRecord(crd_stream, record);

        // Get the type.
        CRDRecordsTypeEnum type = static_cast<CRDRecordsTypeEnum>(record.generic_record_type);

        // Check the errors.
        if(read_result == ReadRecordResultEnum::UNDEFINED_RECORD)
        {
            this->clearCRDContents();
            this->last_read_error_ = ReadFileErrorEnum::UNDEFINED_RECORD;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::UNDEFINED_RECORD;
        }
        else if(type == CRDRecordsTypeEnum::HEADER_RECORD && header_finished)
        {
            this->clearCRDContents();
            this->last_read_error_ = ReadFileErrorEnum::ORDER_ERROR;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::ORDER_ERROR;
        }
        else if(type == CRDRecordsTypeEnum::CFG_RECORD &&  cfg_finished)
        {
            this->clearCRDContents();
            this->last_read_error_ = ReadFileErrorEnum::ORDER_ERROR;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::ORDER_ERROR;
        }
        else if(type == CRDRecordsTypeEnum::DATA_RECORD && data_finished)
        {
            this->clearCRDContents();
            this->last_read_error_ = ReadFileErrorEnum::ORDER_ERROR;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::ORDER_ERROR;
        }
        else if(type == CRDRecordsTypeEnum::EOS_RECORD && eos_finished)
        {
            this->clearCRDContents();
            this->last_read_error_ = ReadFileErrorEnum::MULTIPLE_EOS;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::MULTIPLE_EOS;
        }
        else if(type == CRDRecordsTypeEnum::EOF_RECORD && !eos_finished)
        {
            this->clearCRDContents();
            this->last_read_error_ = ReadFileErrorEnum::EOS_NOT_FOUND;
            return ReadFileErrorEnum::EOS_NOT_FOUND;
        }

        // Check the record type.
        if(type == CRDRecordsTypeEnum::HEADER_RECORD)
        {
             header_vector.push_back(record);
        }
        else if(type == CRDRecordsTypeEnum::CFG_RECORD)
        {
            // Check if we send the header data to the internal structs.
            if(!header_finished)
            {
                // Check if we hava header records.
                if(!header_vector.empty())
                {
                    // Save the possible issues.
                    this->read_header_errors = this->header.readHeader(header_vector);
                    header_finished = true;

                    // Get the version:
                    if(this->header.formatHeader())
                        version = this->header.formatHeader()->crd_version;
                    else
                    {
                        this->clearCRDContents();
                        this->last_read_error_ = ReadFileErrorEnum::VERSION_UNKNOWN;
                        return ReadFileErrorEnum::VERSION_UNKNOWN;
                    }
                }
                else
                {
                    this->clearCRDContents();
                    this->last_read_error_ = ReadFileErrorEnum::NO_HEADER_FOUND;
                    return ReadFileErrorEnum::NO_HEADER_FOUND;
                }
            }

            // Store the configuration record.
            cfg_vector.push_back(record);
        }
        else if(type == CRDRecordsTypeEnum::DATA_RECORD)
        {
            // Check if we send the header data to the internal structs.
            if(!cfg_finished)
            {
                // Check if we hava header records.
                if(!cfg_vector.empty())
                {
                    // Save the possible issues.
                    this->read_cfg_errors = this->configuration.readConfiguration(cfg_vector, version);
                    cfg_finished = true;
                }
                else
                {
                    this->clearCRDContents();
                    this->last_read_error_ = ReadFileErrorEnum::NO_CFG_FOUND;
                    return ReadFileErrorEnum::NO_CFG_FOUND;
                }
            }

            // Store the data record.
            data_vector.push_back(record);
        }
        else if(type == CRDRecordsTypeEnum::EOS_RECORD)
        {
            // Check if we send the header data to the internal structs.
            if(!data_finished)
            {
                // Check if we hava header records.
                if(data_vector.empty())
                {
                    this->clearCRDContents();
                    this->last_read_error_ = ReadFileErrorEnum::NO_DATA_FOUND;
                    return ReadFileErrorEnum::NO_DATA_FOUND;
                }
                else if(!header_finished || !cfg_finished)
                {
                    this->clearCRDContents();
                    this->last_read_error_ = ReadFileErrorEnum::FILE_TRUNCATED;
                    return ReadFileErrorEnum::FILE_TRUNCATED;
                }
                else if(!data_vector.empty())
                {
                    // Save the possible issues.
                    this->read_data_errors = this->data.readData(data_vector, version);
                    data_finished = true;
                }
            }

            // Store the data record.
            eos_finished = true;
        }
        else if(type == CRDRecordsTypeEnum::EOF_RECORD)
        {
            eof_finished = true;
        }

        // Update the read finished variable.
        switch (open_option)
        {
            // Update the finished flag.
            case OpenOptionEnum::ONLY_HEADER:
            read_finished = header_finished;
            break;
            case OpenOptionEnum::ONLY_HEADER_AND_CFG:
            read_finished = (header_finished && cfg_finished);
            break;
            case OpenOptionEnum::ALL_DATA:
            read_finished = (header_finished && cfg_finished && data_finished && eof_finished);
            break;
        }

        // Update the finished variable if the stream is empty.
        read_finished = read_finished || crd_stream.isEmpty() || eof_finished;
    }

    // Check if the stream is not empty.
    if(eof_finished && open_option == OpenOptionEnum::ALL_DATA && !crd_stream.isEmpty())
    {
        //Clear.
        this->clearCRDContents();

        // Get the next line for error storing.
        std::string line;
        std::vector<std::string> tokens;
        crd_stream.getline(line);
        dpslr::common::ConsolidatedRecordStruct rec;
        rec.line_number = crd_stream.getLineNumber();
        rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::UNKNOWN_TYPE;
        if(!line.empty())
        {
            dpslr::helpers::split(tokens, line, " ", false);
            rec.tokens = tokens;
        }

        // Store the last error struct.
        this->last_error_record_ = rec;

        error = ReadFileErrorEnum::CONTENT_AFTER_EOF;
    }
    // Check if the file is truncated.
    else if(!header_finished)
    {
        error = ReadFileErrorEnum::FILE_TRUNCATED;
    }
    else if(!cfg_finished && open_option >= OpenOptionEnum::ONLY_HEADER_AND_CFG)
    {
        error = ReadFileErrorEnum::FILE_TRUNCATED;
    }
    else if(!data_finished && open_option == OpenOptionEnum::ALL_DATA)
    {
        error = ReadFileErrorEnum::FILE_TRUNCATED;
    }
    else if(!eof_finished && open_option == OpenOptionEnum::ALL_DATA)
    {
        error = ReadFileErrorEnum::EOF_NOT_FOUND;
    }
    // Check if we have issues with the internal stored data.
    else if(this->read_header_errors.empty() && this->read_cfg_errors.empty() && this->read_data_errors.empty())
    {
        error = ReadFileErrorEnum::NOT_ERROR;
    }
    else if(!this->read_header_errors.empty() && this->read_cfg_errors.empty() && this->read_data_errors.empty())
    {
        error = ReadFileErrorEnum::HEADER_LOAD_WARNING;
    }
    else if(this->read_header_errors.empty() && !this->read_cfg_errors.empty() && this->read_data_errors.empty())
    {
        error = ReadFileErrorEnum::CFG_LOAD_WARNING;
    }
    else if(this->read_header_errors.empty() && this->read_cfg_errors.empty() && !this->read_data_errors.empty())
    {
        error = ReadFileErrorEnum::DATA_LOAD_WARNING;
    }
    else
    {
        error = ReadFileErrorEnum::RECORDS_LOAD_WARNING;
    }

    // Clear the CRD if neccesary.
    if(error > ReadFileErrorEnum::DATA_LOAD_WARNING)
        this->clearCRDContents();
    else
        this->empty_ = false;

    this->last_read_error_ = error;

    // Return the error.
    return error;
}

CRD::ReadFileErrorEnum CRD::openCRDData()
{
    return this->openCRDFile(this->crd_fullpath, CRD::OpenOptionEnum::ALL_DATA);
}

CRD::WriteFileErrorEnum CRD::writeCRDFile(const std::string &crd_filepath, CRDData::DataGenerationOptionEnum data_opt,
                                         bool force)
{
    std::ifstream input_file(crd_filepath);
    if (input_file.good() && !force)
        return CRD::WriteFileErrorEnum::FILE_ALREADY_EXIST;

    if (!this->header.formatHeader())
        return CRD::WriteFileErrorEnum::VERSION_UNKNOWN;

    // Close the previous.
    input_file.close();

    // Open the file.
    std::ofstream output_file(crd_filepath);

    // Store the records.
    output_file << this->header.generateHeaderLines(true) << std::endl
                << this->configuration.generateConfigurationLines(this->header.formatHeader()->crd_version) << std::endl
                << this->data.generateDataLines(this->header.formatHeader()->crd_version, data_opt) << std::endl
                << this->EndRecordsString[static_cast<int>(CRDRecordsTypeEnum::EOS_RECORD)] << std::endl
                << this->EndRecordsString[static_cast<int>(CRDRecordsTypeEnum::EOF_RECORD)];

    // Close the file.
    output_file.close();

    // Return no error.
    return CRD::WriteFileErrorEnum::NOT_ERROR;
}

CRD::ReadRecordResultEnum CRD::readRecord(dpslr::helpers::InputFileStream& stream,
                                          dpslr::common::ConsolidatedRecordStruct &rec)
{
    // Clear the record.
    rec.clearAll();

    // Check if the file is open.
    if(!stream.is_open())
        return ReadRecordResultEnum::STREAM_NOT_OPEN;

    // Check if the stream is empty.
    if(stream.isEmpty())
        return ReadRecordResultEnum::STREAM_EMPTY;

    // Aux containers and variables.
    constexpr int comment_enum_pos = static_cast<int>(
                dpslr::common::ConsolidatedRecordStruct::CommonRecords::COMMENT_RECORD);

    std::vector<std::string> tokens;
    std::string line;
    bool record_finished = false;

    // Get the record.
    while(!record_finished && stream.getline(line))
    {
        // Always store the line number.
        rec.line_number = stream.getLineNumber();
        rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::UNKNOWN_TYPE;

        // Check if the line is empty.
        if(!line.empty())
        {
            // Get the line and split it to get the tokens.
            dpslr::helpers::split(tokens, line, " ", false);
            tokens[0] = dpslr::helpers::toUpper(tokens[0]);

            // Check the EOS case.
            if(tokens[0] == EndRecordsString[static_cast<int>(CRDRecordsTypeEnum::EOS_RECORD)])
            {
                rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CRD_TYPE;
                rec.tokens = tokens;
                rec.generic_record_type = static_cast<int>(CRDRecordsTypeEnum::EOS_RECORD);
                record_finished = true;
            }
            // Check the EOF case.
            else if(tokens[0] == EndRecordsString[static_cast<int>(CRDRecordsTypeEnum::EOF_RECORD)])
            {
                rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CRD_TYPE;
                rec.tokens = tokens;
                rec.generic_record_type = static_cast<int>(CRDRecordsTypeEnum::EOF_RECORD);
                record_finished = true;
            }
            // Check the comment case.
            else if(tokens[0] == dpslr::common::ConsolidatedRecordStruct::CommonRecordsString[comment_enum_pos])
            {
                // Check the size for empty comments.
               rec.comment_block.push_back((tokens.size() >= 2 ? line.substr(3) : ""));
            }
            // Check the other records case.
            else
            {
                // Find the token id in the containers.
                if(dpslr::helpers::contains(CRDHeader::HeaderLineString, tokens[0]))
                {
                    rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CRD_TYPE;
                    rec.tokens = tokens;
                    rec.generic_record_type = static_cast<int>(CRDRecordsTypeEnum::HEADER_RECORD);
                    record_finished = true;
                }
                else if(dpslr::helpers::contains(CRDConfiguration::CfgLineString, tokens[0]))
                {
                    rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CRD_TYPE;
                    rec.tokens = tokens;
                    rec.line_number = stream.getLineNumber();
                    rec.generic_record_type = static_cast<int>(CRDRecordsTypeEnum::CFG_RECORD);
                    record_finished = true;
                }
                else if(dpslr::helpers::contains(CRDData::DataLineString, tokens[0]))
                {
                    rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CRD_TYPE;
                    rec.tokens = tokens;
                    rec.line_number = stream.getLineNumber();
                    rec.generic_record_type = static_cast<int>(CRDRecordsTypeEnum::DATA_RECORD);
                    record_finished = true;
                }
                else
                {
                    // Store the tokens (for external checking) and return error.
                    rec.tokens = tokens;
                    return ReadRecordResultEnum::UNDEFINED_RECORD;
                }
            } // End check record cases.
        } // End skip empty lines.
    } // End record generation.

    // Check if we have the record finished.
    if(!record_finished)
        return ReadRecordResultEnum::UNDEFINED_RECORD;

    // Return no error.
    return ReadRecordResultEnum::NOT_ERROR;
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================
