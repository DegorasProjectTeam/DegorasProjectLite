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

#include "includes/class_cpf.h"
#include "includes/helpers.h"
#include "includes/utils.h"

#include <array>

// ========== CPF HEADER ===============================================================================================

// --- CPF HEADER CONST EXPRESSIONS ------------------------------------------------------------------------------------
const std::array<std::string, 5> CPFHeader::HeaderLineString {"H1", "H2", "H3", "H4", "H5"};
const std::array<int, 2> CPFHeader::CPFVersions {1, 2};  // Add new main versions here.
// ---------------------------------------------------------------------------------------------------------------------


// --- CPF HEADER CONSTRUCTORS -----------------------------------------------------------------------------------------
CPFHeader::CPFHeader(float cpf_version) :
    basic_info1_header(BasicInfo1Header())
{
    this->basic_info1_header->cpf_version = cpf_version;
}

// ---------------------------------------------------------------------------------------------------------------------

// --- CPF HEADER CLEAR FUNCTIONS --------------------------------------------------------------------------------------
void CPFHeader::clearAll()
{
    // Clear all records
    this->clearBasicInfo1Header();
    this->clearBasicInfo2Header();
    this->clearExpectedAccuracyHeader();
    this->clearTransponderInfoHeader();
    this->clearCoMCorrectionHeader();
}

void CPFHeader::clearBasicInfo1Header() {this->basic_info1_header = {};}

void CPFHeader::clearBasicInfo2Header() {this->basic_info2_header = {};}

void CPFHeader::clearExpectedAccuracyHeader() {this->exp_accuracy_header = {};}

void CPFHeader::clearTransponderInfoHeader() {this->transp_info_header = {};}

void CPFHeader::clearCoMCorrectionHeader() {this->com_corr_header = {};}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF HEADER GETTERS ----------------------------------------------------------------------------------------------
const dpslr::common::optional<CPFHeader::BasicInfo1Header> &CPFHeader::basicInfo1Header() const
{return this->basic_info1_header;}

const dpslr::common::optional<CPFHeader::BasicInfo2Header> &CPFHeader::basicInfo2Header() const
{return this->basic_info2_header;}

const dpslr::common::optional<CPFHeader::ExpectedAccuracyHeader> &CPFHeader::expectedAccuracyHeader() const
{return this->exp_accuracy_header;}

const dpslr::common::optional<CPFHeader::TransponderInfoHeader> &CPFHeader::transponderInfoHeader() const
{return this->transp_info_header;}

const dpslr::common::optional<CPFHeader::CoMCorrectionHeader> &CPFHeader::coMCorrectionHeader() const
{return this->com_corr_header;}

dpslr::common::optional<CPFHeader::BasicInfo1Header> &CPFHeader::basicInfo1Header()
{return this->basic_info1_header;}

dpslr::common::optional<CPFHeader::BasicInfo2Header> &CPFHeader::basicInfo2Header()
{return this->basic_info2_header;}

dpslr::common::optional<CPFHeader::ExpectedAccuracyHeader> &CPFHeader::expectedAccuracyHeader()
{return this->exp_accuracy_header;}

dpslr::common::optional<CPFHeader::TransponderInfoHeader> &CPFHeader::transponderInfoHeader()
{return this->transp_info_header;}

dpslr::common::optional<CPFHeader::CoMCorrectionHeader> &CPFHeader::coMCorrectionHeader()
{return this->com_corr_header;}

// ---------------------------------------------------------------------------------------------------------------------

// --- CPF HEADER SETTERS ----------------------------------------------------------------------------------------------
void CPFHeader::setBasicInfo1Header(const BasicInfo1Header &bi1h) {this->basic_info1_header = bi1h;}

void CPFHeader::setBasicInfo2Header(const BasicInfo2Header &bi2h) {this->basic_info2_header = bi2h;}

void CPFHeader::setExpectedAccuracyHeader(const ExpectedAccuracyHeader &eah) {this->exp_accuracy_header = eah;}

void CPFHeader::setTransponderInfoHeader(const TransponderInfoHeader &tih) {this->transp_info_header = tih;}

void CPFHeader::setCoMCorrection(const CoMCorrectionHeader &comh) {this->com_corr_header = comh;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF HEADER LINES GENERATOR --------------------------------------------------------------------------------------
std::string CPFHeader::generateHeaderLines()
{
    // TODO: hacer antes de llamar a esta funcion una funcion de chequeo que compruebe la integridad de esto.

    // Header line
    std::stringstream header_line;

    // TODO: lines are not generated if there is no basic info 1 header
    if (!this->basic_info1_header)
        return "";

    // For v1 and 2.
    if(this->basic_info1_header->cpf_version >= 1 && this->basic_info1_header->cpf_version <=3)
    {
        // Basic info 1 header.
        if(this->basic_info1_header)
        {
            // Add the associated comment block.
            if(!this->basic_info1_header->comment_block.empty())
                header_line << this->basic_info1_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->basic_info1_header->generateLine() << std::endl;
        }

        // Basic info 2 header.
        if(this->basic_info2_header)
        {
            // Add the associated comment block.
            if(!this->basic_info2_header->comment_block.empty())
                header_line << this->basic_info2_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->basic_info2_header->generateLine(this->basic_info1_header->cpf_version) << std::endl;
        }

        // Expected accuracy header.
        if(this->exp_accuracy_header)
        {
            // Add the associated comment block.
            if(!this->exp_accuracy_header->comment_block.empty())
                header_line << this->exp_accuracy_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->exp_accuracy_header->generateLine(this->basic_info1_header->cpf_version) << std::endl;
        }

        // Transponder info header.
        if(this->transp_info_header)
        {
            // Add the associated comment block.
            if(!this->transp_info_header->comment_block.empty())
                header_line << this->transp_info_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->transp_info_header->generateLine(this->basic_info1_header->cpf_version) << std::endl;
        }

        // CoM correction info header.
        if(this->com_corr_header)
        {
            // Add the associated comment block.
            if(!this->com_corr_header->comment_block.empty())
                header_line << this->com_corr_header->generateCommentBlock() << std::endl;
            // Add the line.
            header_line << this->com_corr_header->generateLine(this->basic_info1_header->cpf_version) << std::endl;
        }
    }

    // Return all the lines.
    return header_line.str().substr(0, header_line.str().find_last_of('\n'));
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF HEADER PUBLIC READER FUNCTIONS ------------------------------------------------------------------------------

// Generic reading header function.
dpslr::common::RecordReadErrorMultimap CPFHeader::readHeader(const dpslr::common::RecordLinesVector &rec_v)
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
dpslr::common::RecordReadErrorEnum CPFHeader::readBasicInfo1Header(const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens: H1, CPF, VERSION, SOURCE, YEAR, MONTH, DAY, HOUR, SEQ NUMBER,
    //        SUB-DAILY SEQ NUMBER [only for v2], TARGET NAME, NOTES
    // This read MUST be ok, because we always need the version for reading the next data.

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Variables.
    std::tm date_time;
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_ERROR;

    std::string aux = dpslr::helpers::toUpper(tokens[0]);


    // Check if the record type is correct.
    if (dpslr::helpers::toUpper(tokens[0]) !=
             HeaderLineString[static_cast<int>(HeaderRecordEnum::BASIC_INFO_1_HEADER)])
        result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    else
    {
        try
        {
            // New format header struct.
            BasicInfo1Header bi1h;

            // Get the line and version.
            bi1h.cpf_version = std::stof(tokens[2]);

            // Check the data size for each version. Notes field is optional
            // For v1.
            if (bi1h.cpf_version >= 1 && bi1h.cpf_version < 2 && tokens.size() < 10)
                result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
            // For v2.
            else if(bi1h.cpf_version >= 2 && bi1h.cpf_version < 3 && tokens.size() != 11 && tokens.size() != 12)
                result = dpslr::common::RecordReadErrorEnum::BAD_SIZE;
            else
            {
                // TODO: version with decimals?
                auto it = std::find(CPFVersions.begin(), CPFVersions.end(), static_cast<int>(bi1h.cpf_version));
                if(it == CPFVersions.end())
                {
                    result = dpslr::common::RecordReadErrorEnum::VERSION_MISMATCH;
                }
                else
                {
                    bi1h.cpf_source = tokens[3];
                    // Get the file creation time UTC.
                    date_time.tm_year = std::stoi(tokens[4]) - 1900;
                    date_time.tm_mon = std::stoi(tokens[5]) - 1;
                    date_time.tm_mday = std::stoi(tokens[6]);
                    date_time.tm_hour = std::stoi(tokens[7]);
                    date_time.tm_min = 0;
                    date_time.tm_sec = 0;
                    date_time.tm_isdst = 0;
                    bi1h.cpf_production_date = std::chrono::system_clock::from_time_t(MKGMTIME(&date_time));
                    bi1h.cpf_sequence_number = std::stoi(tokens[8]);

                    if (bi1h.cpf_version >= 1.f && bi1h.cpf_version < 2.f)
                    {
                        // Target name and notes can have spaces
                        std::string last;
                        unsigned int i = 9;
                        while (i < tokens.size())
                        {
                            last.insert(last.end(), tokens[i].cbegin(), tokens[i].cend());
                            i++;
                        }
                        bi1h.target_name = last.substr(0, 10);
                        if (last.size() > 11)
                            bi1h.cpf_notes = last.substr(11, 10);
                    }
                    else
                    {
                        bi1h.cpf_subsequence_number = std::stoi(tokens[9]);
                        bi1h.target_name = tokens[10];
                        if (tokens.size() > 11)
                            bi1h.cpf_notes = tokens[11];
                    }

                    // Add the associated comments, the line number, and the tokens.
                    bi1h.comment_block = record.comment_block;
                    bi1h.line_number = record.line_number;
                    bi1h.tokens = record.tokens;

                    // Finally, store header if no converssion error ocurred
                    this->basic_info1_header = std::move(bi1h);
                }
            }

        } catch (...)
        {
            result = dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }
    }

    // If there was any error, clear format header
    if (dpslr::common::RecordReadErrorEnum::NOT_ERROR != result)
        this->clearBasicInfo1Header();

    // Return the result.
    return result;
}

// Specific function for reading H2.
dpslr::common::RecordReadErrorEnum CPFHeader::readBasicInfo2Header(const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens: H2, ILRSID, SIC, Norad, Start Year, Start Month, Start Day, Start Hour, Start Minute, Start Second,
    //         End Year, End Month, End Day, End Hour, End Minute, End Second, Time between entries, TIV compatibility,
    //         Target class, Reference frame, Rotational angle type, CoM correction applied, Target dynamics

    // Delete the current data.
    this->clearBasicInfo2Header();

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check if format header is ok
    if(!this->basic_info1_header)
        return dpslr::common::RecordReadErrorEnum::VERSION_UNKNOWN;

    // Check the data size for each version.
    // For v1.
    if (this->basic_info1_header->cpf_version >= 1 && this->basic_info1_header->cpf_version < 2 && tokens.size() != 22)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // For v2.
    else if(this->basic_info1_header->cpf_version >= 2 &&
            this->basic_info1_header->cpf_version < 3 && tokens.size() != 23)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) !=
             HeaderLineString[static_cast<int>(HeaderRecordEnum::BASIC_INFO_2_HEADER)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    try
    {
        // New station header struct.
        BasicInfo2Header bi2h;
        std::tm date_time;

        // Get the data.
        bi2h.id = tokens[1];
        if ("-1" != tokens[2])
            bi2h.sic = tokens[2];
        // TODO: sometimes Norad comes with leading zeroes. We will trim them. Norad 0 is not valid.
        auto norad = tokens[3];
        norad.erase(0, norad.find_first_not_of('0'));
        bi2h.norad = norad;
        // Get the file start time UTC.
        date_time.tm_year = std::stoi(tokens[4]) - 1900;
        date_time.tm_mon = std::stoi(tokens[5]) - 1;
        date_time.tm_mday = std::stoi(tokens[6]);
        date_time.tm_hour = std::stoi(tokens[7]);
        date_time.tm_min = std::stoi(tokens[8]);
        date_time.tm_sec = std::stoi(tokens[9]);
        date_time.tm_isdst = 0;
        bi2h.start_time = std::chrono::system_clock::from_time_t(MKGMTIME(&date_time));

        // Get the file end time UTC.
        date_time.tm_year = std::stoi(tokens[10]) - 1900;
        date_time.tm_mon = std::stoi(tokens[11]) - 1;
        date_time.tm_mday = std::stoi(tokens[12]);
        date_time.tm_hour = std::stoi(tokens[13]);
        date_time.tm_min = std::stoi(tokens[14]);
        date_time.tm_sec = std::stoi(tokens[15]);
        date_time.tm_isdst = 0;
        bi2h.end_time = std::chrono::system_clock::from_time_t(MKGMTIME(&date_time));

        bi2h.time_between_entries = std::chrono::seconds(std::stoi(tokens[16]));
        bi2h.tiv_compatible = "1" == tokens[17];
        bi2h.target_class = static_cast<TargetClassEnum>(std::stoi(tokens[18]));
        bi2h.reference_frame = static_cast<ReferenceFrameEnum>(std::stoi(tokens[19]));
        bi2h.rot_angle_type = static_cast<RotAngleTypeEnum>(std::stoi(tokens[20]));
        bi2h.com_applied = "1" == tokens[21];

        // Only for v2.
        if(this->basic_info1_header->cpf_version >= 2 && this->basic_info1_header->cpf_version < 3)
            bi2h.target_dynamics = static_cast<TargetDynamicsEnum>(std::stoi(tokens[22]));

        // Add the associated comments, the line number, and the tokens.
        bi2h.comment_block = record.comment_block;
        bi2h.line_number = record.line_number;
        bi2h.tokens = record.tokens;

        // If there was no error at reading, store header
        this->basic_info2_header = std::move(bi2h);

    } catch (...)
    {
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

// Specific function for reading H3.
dpslr::common::RecordReadErrorEnum CPFHeader::readExpectedAccuracyHeader(
        const dpslr::common::ConsolidatedRecordStruct& )
{
    // TODO
    return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
}

// Specific function for reading H4.
dpslr::common::RecordReadErrorEnum CPFHeader::readTransponderInfoHeader(
        const dpslr::common::ConsolidatedRecordStruct& )
{
    // TODO
    return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
}

// Specific function for reading H5.
dpslr::common::RecordReadErrorEnum CPFHeader::readCoMCorrectionHeader(
        const dpslr::common::ConsolidatedRecordStruct& record)
{
    // Tokens are: H5, CoM correction

    // Clear current data.
    this->clearCoMCorrectionHeader();

    // Get the tokens.
    std::vector<std::string> tokens = record.tokens;

    // Check if size is correct.
    if (tokens.size() != 2)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (dpslr::helpers::toUpper(tokens[0]) !=
             HeaderLineString[static_cast<int>(HeaderRecordEnum::COM_CORRECTION_HEADER)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;
    // All ok at this momment.
    try
    {
        // New session header struct.
        CoMCorrectionHeader comh;

        // Get prediction type.
        comh.com_correction = std::stod(tokens[1]);

        // Add the associated comments, the line number, and the tokens.
        comh.comment_block = record.comment_block;
        comh.line_number = record.line_number;
        comh.tokens = record.tokens;

        // If there was no error at reading, store header
        this->com_corr_header = std::move(comh);

    } catch (...)
    {
        return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
    }

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}


// --- CPF HEADER PRIVATE FUNCTIONS  -----------------------------------------------------------------------------------
dpslr::common::RecordReadErrorEnum CPFHeader::readHeaderLine(const dpslr::common::RecordLinePair &lpair)
{
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;

    // Select the specific read funtion for each line.
    switch (static_cast<HeaderRecordEnum>(lpair.first))
    {
        case HeaderRecordEnum::BASIC_INFO_1_HEADER:
        result = this->readBasicInfo1Header(lpair.second);
        break;

        case HeaderRecordEnum::BASIC_INFO_2_HEADER:
        result = this->readBasicInfo2Header(lpair.second);
        break;

        case HeaderRecordEnum::EXPECTED_ACCURACY_HEADER:
        result = this->readExpectedAccuracyHeader(lpair.second);
        break;

        case HeaderRecordEnum::TRANSPONDER_INFO_HEADER:
        result = this->readTransponderInfoHeader(lpair.second);
        break;

        case HeaderRecordEnum::COM_CORRECTION_HEADER:
        result = this->readCoMCorrectionHeader(lpair.second);
        break;
    }

    return result;
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF HEADERS STRUCTS ---------------------------------------------------------------------------------------------
std::string CPFHeader::BasicInfo1Header::generateLine()
{
    // Base line.
    std::stringstream line_h1;

    // Get the creation time (UTC).
    std::tm *time;
    dpslr::common::HRTimePoint timepoint = dpslr::common::HRTimePoint::clock::now();
    std::time_t datetime = std::chrono::system_clock::to_time_t(timepoint);
    time = std::gmtime(&datetime);

    // Update the production date in the class.
    this->cpf_production_date = timepoint;

    // Generate H1 for version 1 (fixed format).
    if (this->cpf_version >= 1 && this->cpf_version < 2)
    {
        line_h1 << std::fixed << std::right
                << "H1 CPF"
                << std::setw(3) << static_cast<int>(this->cpf_version)
                << "  " << this->cpf_source.substr(0,3)
                << std::setw(5) << (time->tm_year + 1900)
                << std::setw(3) << (time->tm_mon + 1)
                << std::setw(3) << (time->tm_mday)
                << std::setw(3) << (time->tm_hour)
                << std::setw(6) << this->cpf_sequence_number
                << ' ' << this->target_name.substr(0, 10)
                << ' ' << this->cpf_notes.substr(0, 10);
    }

    // Generate H1 for version 2 (free format).
    if (this->cpf_version >= 2  && this->cpf_version < 3)
    {
        line_h1 << "H1 CPF"
                << ' ' << this->cpf_version          // Free format, so we could print "2.0", "2.1", etc.
                << ' ' << this->cpf_source
                << ' ' << (time->tm_year + 1900)
                << ' ' << (time->tm_mon + 1)
                << ' ' << (time->tm_mday)
                << ' ' << (time->tm_hour)
                << ' ' << this->cpf_sequence_number
                << ' ' << this->cpf_subsequence_number
                << ' ' << this->target_name
                << ' ' << this->cpf_notes;
    }

    // Return the H1
    return line_h1.str();
}

std::string CPFHeader::BasicInfo2Header::generateLine(float version) const
{
    // H2 Base line.
    std::stringstream line_h2;

    // Get the session times.
    std::time_t start = std::chrono::system_clock::to_time_t(this->start_time);
    std::time_t end = std::chrono::system_clock::to_time_t(this->end_time);
    std::tm start_tm(*std::gmtime(&start));
    std::tm end_tm(*std::gmtime(&end));

    // For version 1 (fixed format).
    if (version >= 1 && version < 2)
    {
        // TODO: name should be lowercase only in ILRS or always?
        line_h2 << std::fixed << std::left
                << "H2" << std::right
                << std::setw(9)  << this->id.substr(0, 8)
                << std::setw(5)  << (this->sic ? this->sic->substr(0, 4) : "9999")
                << std::setw(9)  << (this->norad.empty() ? "99999999" : this->norad.substr(0, 8))
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
                << std::setw(6) << this->time_between_entries.count()
                << std::setw(2) << (this->tiv_compatible ? 1 : 0)
                << std::setw(2) << static_cast<int>(this->target_class)
                << std::setw(3) << static_cast<int>(this->reference_frame)
                << std::setw(2) << static_cast<int>(this->rot_angle_type)
                << std::setw(2) << (this->com_applied ? 1 : 0);
    }

    // For v2 only (free format).
    if(version >= 2 && version < 3)
    {
        line_h2 << std::fixed
                << "H2"
                << ' ' << this->id
                << ' ' << (this->sic ? *this->sic : "na")
                << ' ' << (this->norad.empty() ? "na" : this->norad)
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
                << ' ' << this->time_between_entries.count()
                << ' ' << (this->tiv_compatible ? 1 : 0)
                << ' ' << static_cast<int>(this->target_class)
                << ' ' << static_cast<int>(this->reference_frame)
                << ' ' << static_cast<int>(this->rot_angle_type)
                << ' ' << (this->com_applied ? 1 : 0)
                << ' ' << static_cast<int>(this->target_dynamics);
    }

    // Return the H2
    return line_h2.str();
}

std::string CPFHeader::ExpectedAccuracyHeader::generateLine(float) const
{
    // TODO
    return "";
}

std::string CPFHeader::TransponderInfoHeader::generateLine(float) const
{
    // TODO
    return "";
}

std::string CPFHeader::CoMCorrectionHeader::generateLine(float version) const
{
    // H5 Base line.
    std::stringstream line_h5;

    // For version 1 (fixed format).
    if (version >= 1 && version < 2)
    {
        line_h5 << std::fixed << std::left
                << "H5 " << dpslr::helpers::numberToStr(this->com_correction, 6, 4);
    }

    // For v2 only (free format).
    if(version >= 2 && version < 3)
    {
        line_h5 << std::fixed
                << "H5"
                << ' ' << this->com_correction;
    }

    // Return the H5
    return line_h5.str();
}
// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================

// ========== CPF DATA =================================================================================================

// --- CPF DATA CONST EXPRESSIONS --------------------------------------------------------------------------------------
const std::array<const char*, 7> CPFData::DataLineString {"10", "20", "30", "40", "50", "60", "70"};
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF HEADER CLEAR FUNCTIONS --------------------------------------------------------------------------------------
void CPFData::clearAll()
{
    this->clearPositionRecords();
    this->clearVelocityRecords();
    this->clearCorrectionRecords();
    this->clearTransponderRecords();
    this->clearOffsetFromCenterRecords();
    this->clearRotationAngleRecords();
    this->clearEarthOrientationRecords();
}

void CPFData::clearPositionRecords() {this->pos_records.clear();}

void CPFData::clearVelocityRecords() {this->vel_records.clear();}

void CPFData::clearCorrectionRecords() {this->corr_records.clear();}

void CPFData::clearTransponderRecords() {this->transp_records.clear();}

void CPFData::clearOffsetFromCenterRecords() {this->offset_records.clear();}

void CPFData::clearRotationAngleRecords() {this->rot_angle_records = {};}

void CPFData::clearEarthOrientationRecords() {this->earth_orientation_records = {};}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF DATA SETTERS ------------------------------------------------------------------------------------------------

void CPFData::setPositionRecords(const std::vector<PositionRecord> &records) {this->pos_records = records;}

void CPFData::setVelocityRecords(const std::vector<VelocityRecord> &records) {this->vel_records = records;}

void CPFData::setCorrectionsRecords(const std::vector<CorrectionsRecord> &records) {this->corr_records = records;}

void CPFData::setTransponderRecords(const std::vector<TransponderRecord> &records) {this->transp_records = records;}

void CPFData::setOffsetFromCenterRecords(const std::vector<OffsetFromCenterRecord> &records)
{this->offset_records = records;}

void CPFData::setRotationAngleRecords(const std::vector<RotationAngleRecord> &records)
{this->rot_angle_records = records;}

void CPFData::setEarthOrientationRecords(const std::vector<EarthOrientationRecord> &records)
{this->earth_orientation_records = records;}

void CPFData::addPositionRecord(const PositionRecord &rec) {this->pos_records.push_back(rec);}

void CPFData::addVelocityRecord(const VelocityRecord &rec) {this->vel_records.push_back(rec);}

void CPFData::addCorrectionsRecord(const CorrectionsRecord &rec) {this->corr_records.push_back(rec);}

void CPFData::addTransponderRecord(const TransponderRecord &rec) {this->transp_records.push_back(rec);}

void CPFData::addOffsetFromCenterRecord(const OffsetFromCenterRecord &rec) {this->offset_records.push_back(rec);}

void CPFData::addRotationAngleRecord(const RotationAngleRecord &rec) {this->rot_angle_records.push_back(rec);}

void CPFData::addEarthOrientationRecord(const EarthOrientationRecord &rec)
{this->earth_orientation_records.push_back(rec);}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF DATA GETTERS ------------------------------------------------------------------------------------------------
const std::vector<CPFData::PositionRecord> &CPFData::positionRecords() const {return this->pos_records;}

const std::vector<CPFData::VelocityRecord> &CPFData::velocityRecords() const {return this->vel_records;}

const std::vector<CPFData::CorrectionsRecord> &CPFData::correctionsRecords() const {return this->corr_records;}

const std::vector<CPFData::TransponderRecord> &CPFData::transponderRecords() const {return this->transp_records;}

const std::vector<CPFData::OffsetFromCenterRecord> &CPFData::offsetFromCenterRecords() const
{return this->offset_records;}

const std::vector<CPFData::RotationAngleRecord> &CPFData::rotationAngleRecords() const {return this->rot_angle_records;}

const std::vector<CPFData::EarthOrientationRecord> &CPFData::earthOrientationRecords() const
{return this->earth_orientation_records;}

// ---------------------------------------------------------------------------------------------------------------------

// --- CPF NON-CONST DATA GETTERS ------------------------------------------------------------------------------------------------

std::vector<CPFData::PositionRecord> &CPFData::positionRecords() {return this->pos_records;}

std::vector<CPFData::VelocityRecord> &CPFData::velocityRecords() {return this->vel_records;}

std::vector<CPFData::CorrectionsRecord> &CPFData::correctionsRecords() {return this->corr_records;}

std::vector<CPFData::TransponderRecord> &CPFData::transponderRecords() {return this->transp_records;}

std::vector<CPFData::OffsetFromCenterRecord> &CPFData::offsetFromCenterRecords() {return this->offset_records;}

std::vector<CPFData::RotationAngleRecord> &CPFData::rotationAngleRecords() {return this->rot_angle_records;}

std::vector<CPFData::EarthOrientationRecord> &CPFData::earthOrientationRecords()
{return this->earth_orientation_records;}

// ---------------------------------------------------------------------------------------------------------------------

// --- CPF GENERATE DATA LINES -----------------------------------------------------------------------------------------
std::string CPFData::generateDataLines(float version) const
{
    // Data lines
    std::stringstream data;

    // TODO: Lines order:
    if (!this->pos_records.empty())
        data << this->generatePositionRecordsLines(version);

    // TODO: the rest of the lines
    // TODO: do not insert breakline at the end

    // Return all the lines.
    return data.str();
}

std::string CPFData::generatePositionRecordsLines(float version) const
{
    // Position lines
    std::stringstream pr_line;

    // Generate the Position record line for each record.
    for(const auto& pr : this->pos_records)
    {
        // Add the comment block.
        if(!pr.comment_block.empty())
            pr_line << pr.generateCommentBlock() << std::endl;
        pr_line << pr.generateLine(version) << std::endl;
    }

    // Return all the lines.
    return pr_line.str().substr(0, pr_line.str().find_last_of('\n'));
}

std::string CPFData::generateVelocityRecordsLines(float) const
{
    // TODO
    return "";
}

std::string CPFData::generateCorrectionsRecordsLines(float) const
{
    // TODO
    return "";
}

std::string CPFData::generateTransponderRecordsLines(float) const
{
    // TODO
    return "";
}

std::string CPFData::generateOffsetFromCenterRecordsLine(float) const
{
    // TODO
    return "";
}

std::string CPFData::generateRotationAngleRecordsLines(float) const
{
    // TODO
    return "";
}

std::string CPFData::generateEarthOrientationRecordsLine(float) const
{
    // TODO
    return "";
}

// ---------------------------------------------------------------------------------------------------------------------

// --- CPF READ DATA LINES ---------------------------------------------------------------------------------------------
dpslr::common::RecordReadErrorEnum CPFData::readDataLine(const dpslr::common::RecordLinePair &lpair, float version)
{
    // TODO: some of these things are common with all structs and with CRD. Maybe ConsolidatedRecord could include
    // a readfunction that receives a RecordLinePair and version?
    dpslr::common::RecordReadErrorEnum result = dpslr::common::RecordReadErrorEnum::BAD_TYPE;

    switch (static_cast<DataRecordTypeEnum>(lpair.first))
    {
        case DataRecordTypeEnum::POSITION_RECORD:
        result = this->readPositionRecord(lpair.second, version);
        break;

        case DataRecordTypeEnum::VELOCITY_RECORD:
        result = this->readVelocityRecord(lpair.second, version);
        break;

        case DataRecordTypeEnum::CORRECTIONS_RECORD:
        result = this->readCorrectionsRecord(lpair.second, version);
        break;

        case DataRecordTypeEnum::TRANSPONDER_RECORD:
        result = this->readTransponderRecord(lpair.second, version);
        break;

        case DataRecordTypeEnum::OFFSET_FROM_CENTER_RECORD:
        result = this->readOffsetFromCenterRecord(lpair.second, version);
        break;

        case DataRecordTypeEnum::ROT_ANGLE_RECORD:
        result = this->readRotationAngleRecord(lpair.second, version);
        break;

        case DataRecordTypeEnum::EARTH_ORIENTATION_RECORD:
        result = this->readEarthOrientationRecord(lpair.second, version);
        break;
    }

    return result;
}

dpslr::common::RecordReadErrorMultimap CPFData::readData(const dpslr::common::RecordLinesVector &rec_v, float version)
{
    // TODO: this function is common. It could be somehow inherited or shared
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

dpslr::common::RecordReadErrorEnum CPFData::readPositionRecord(
        const dpslr::common::ConsolidatedRecordStruct &rec, float v)
{
    PositionRecord pos_record;

    // Get the tokens.
    std::vector<std::string> tokens = rec.tokens;

    // Check the data size for each version.
    // For v1 and v2
    if (v >= 1 && v < 3 && tokens.size() != 8)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (tokens[0] != DataLineString[static_cast<int>(DataRecordTypeEnum::POSITION_RECORD)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;

    // All ok at this momment.
    else
        try
        {
            // Get the data.
            pos_record.dir_flag = static_cast<CPFData::DirectionFlagEnum>(std::stoi(tokens[1]));
            pos_record.mjd = std::stoi(tokens[2]);
            pos_record.sod = std::stold(tokens[3]);
            pos_record.leap_second = std::stoi(tokens[4]);
            pos_record.geocentric_pos = {std::stold(tokens[5]), std::stold(tokens[6]), std::stold(tokens[7])};

        } catch (...)
        {
            return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // All ok, insert the struct.
    this->pos_records.push_back(pos_record);

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

dpslr::common::RecordReadErrorEnum CPFData::readVelocityRecord(
        const dpslr::common::ConsolidatedRecordStruct &rec, float v)
{
    // Struct.
    VelocityRecord vel_record;

    // Get the tokens.
    std::vector<std::string> tokens = rec.tokens;

    // Check the data size for each version.
    // For v1 and v2.
    if (v >= 1 && v < 3 && tokens.size() != 5)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (tokens[0] != DataLineString[static_cast<int>(DataRecordTypeEnum::VELOCITY_RECORD)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;

    // All ok at this momment.
    else
        try
        {
            // Get the data.
            vel_record.dir_flag = static_cast<CPFData::DirectionFlagEnum>(std::stoi(tokens[1]));
            vel_record.geocentric_vel = {std::stold(tokens[2]), std::stold(tokens[3]), std::stold(tokens[4])};

        } catch (...)
        {
            return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // All ok, insert the struct.
    this->vel_records.push_back(vel_record);

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

dpslr::common::RecordReadErrorEnum CPFData::readCorrectionsRecord(
        const dpslr::common::ConsolidatedRecordStruct &rec, float v)
{
    // Struct.
    CorrectionsRecord corr_record;

    // Get the tokens.
    std::vector<std::string> tokens = rec.tokens;

    // Check the data size for each version.
    // For v1 and v2.
    if (v >= 1 && v < 3 && tokens.size() != 6)
        return dpslr::common::RecordReadErrorEnum::BAD_SIZE;
    // Check if the record type is correct.
    else if (tokens[0] != DataLineString[static_cast<int>(DataRecordTypeEnum::CORRECTIONS_RECORD)])
        return dpslr::common::RecordReadErrorEnum::BAD_TYPE;

    // All ok at this momment.
    else
        try
        {
            // Get the data.
            corr_record.dir_flag = static_cast<CPFData::DirectionFlagEnum>(std::stoi(tokens[1]));
            corr_record.aberration_correction = {std::stold(tokens[2]), std::stold(tokens[3]), std::stold(tokens[4])};
            corr_record.range_correction = std::stod(tokens[5]);

        } catch (...)
        {
            return dpslr::common::RecordReadErrorEnum::CONVERSION_ERROR;
        }

    // All ok, insert the struct.
    this->corr_records.push_back(corr_record);

    // Return the result.
    return dpslr::common::RecordReadErrorEnum::NOT_ERROR;
}

dpslr::common::RecordReadErrorEnum CPFData::readTransponderRecord(const dpslr::common::ConsolidatedRecordStruct &, float)
{
    // TODO
    return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
}

dpslr::common::RecordReadErrorEnum CPFData::readOffsetFromCenterRecord(
        const dpslr::common::ConsolidatedRecordStruct &, float)
{
    // TODO
    return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
}

dpslr::common::RecordReadErrorEnum CPFData::readRotationAngleRecord(
        const dpslr::common::ConsolidatedRecordStruct &, float)
{
    // TODO
    return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
}

dpslr::common::RecordReadErrorEnum CPFData::readEarthOrientationRecord(
        const dpslr::common::ConsolidatedRecordStruct &, float)
{
    // TODO
    return dpslr::common::RecordReadErrorEnum::NOT_IMPLEMENTED;
}

// ---------------------------------------------------------------------------------------------------------------------

// --- CPF DATA STRUCTS ------------------------------------------------------------------------------------------------
std::string CPFData::PositionRecord::generateLine(float) const
{
    // Base line.
    std::stringstream line_10;

    line_10 << "10"
            << ' ' << static_cast<int>(this->dir_flag)
            << ' ' << this->mjd
            << ' ' << dpslr::helpers::numberToStr(this->sod, 13, 6)
            << ' ' << this->leap_second
            << ' ' << dpslr::helpers::numberToStr(this->geocentric_pos[0], 17, 3)
            << ' ' << dpslr::helpers::numberToStr(this->geocentric_pos[1], 17, 3)
            << ' ' << dpslr::helpers::numberToStr(this->geocentric_pos[2], 17, 3);

    // Return LINE 10.
    return line_10.str();
}

std::string CPFData::VelocityRecord::generateLine(float) const
{
    // TODO
    return "";
}

std::string CPFData::CorrectionsRecord::generateLine(float) const
{
    // TODO
    return "";
}

std::string CPFData::TransponderRecord::generateLine(float) const
{
    // TODO
    return "";
}

std::string CPFData::OffsetFromCenterRecord::generateLine(float) const
{
    // TODO
    return "";
}

std::string CPFData::RotationAngleRecord::generateLine(float) const
{
    // TODO
    return "";
}

std::string CPFData::EarthOrientationRecord::generateLine(float) const
{
    // TODO
    return "";
}

// ---------------------------------------------------------------------------------------------------------------------

// =====================================================================================================================

// ========== CPF ======================================================================================================

const std::array<const char*, 2> CPF::EndRecordsString{"H9", "99"};

// --- CPF CONSTRUCTORS ------------------------------------------------------------------------------------------------
CPF::CPF(float version) :
    empty_(false)
{
    // Set the version and creation time at Format Header
    this->header_.basicInfo1Header()->cpf_version = version;
    this->header_.basicInfo1Header()->cpf_production_date = dpslr::common::HRTimePoint::clock::now();
}

CPF::CPF(const std::string &cpf_filepath, OpenOptionEnum option)
{
    this->openCPFFile(cpf_filepath, option);
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF CLEAR METHODS -----------------------------------------------------------------------------------------------
void CPF::clearCPF()
{
    // Clear the CPF contents and leave it empty
    this->clearCPFContents();

    // Clear the error storage.
    this->last_read_error_ = ReadFileErrorEnum::NOT_ERROR;
    this->last_read_error_ = {};
    this->read_header_errors_.clear();
    this->read_data_errors_.clear();

    // Clear others.
    this->cpf_filename_.clear();
    this->cpf_fullpath_.clear();
}

void CPF::clearCPFContents()
{
    // Clear header and data and leave CPF empty
    this->header_.clearAll();
    this->data_.clearAll();

    this->empty_ = true;
}

void CPF::clearCPFHeader()
{
   this->header_.clearAll();
}

void CPF::clearCPFData()
{
    this->data_.clearAll();
}

// ---------------------------------------------------------------------------------------------------------------------

// --- CPF GETTERS -----------------------------------------------------------------------------------------------------
CPFHeader &CPF::getHeader() {return this->header_;}

CPFData &CPF::getData() {return this->data_;}

const CPFHeader &CPF::getHeader() const {return this->header_;}

const CPFData &CPF::getData() const {return this->data_;}

const dpslr::common::RecordReadErrorMultimap &CPF::getReadHeaderErrors() const {return this->read_header_errors_;}

const dpslr::common::RecordReadErrorMultimap &CPF::getReadDataErrors() const {return this->read_data_errors_;}

CPF::ReadFileErrorEnum CPF::getLastReadError() const {return this->last_read_error_;}

const dpslr::common::optional<dpslr::common::ConsolidatedRecordStruct> &CPF::getLastReadErrorRecord() const
{
    return this->last_error_record_;
}

const std::string &CPF::getSourceFilename() const {return this->cpf_filename_;}

const std::string &CPF::getSourceFilepath() const {return this->cpf_fullpath_;}

std::string CPF::getStandardFilename(TargetIdOptionEnum option) const
{
    // Variables.
    std::string filename;

    // Check the preconditions.
    if(!this->header_.basicInfo1Header() || !this->header_.basicInfo2Header())
        return "";

    // Get the target identifier.
    switch (option)
    {
        case TargetIdOptionEnum::ILRS_ID:
        filename.append(this->header_.basicInfo2Header()->id);
        break;

        case TargetIdOptionEnum::SHORT_COSPAR:
        filename.append(dpslr::utils::ilrsidToShortcospar(this->header_.basicInfo2Header()->id));
        break;

        case TargetIdOptionEnum::COSPAR:
        filename.append(dpslr::utils::ilrsidToCospar(this->header_.basicInfo2Header()->id));
        break;

        case TargetIdOptionEnum::NORAD:
        filename.append(this->header_.basicInfo2Header()->norad);
        break;

        case TargetIdOptionEnum::TARGET_NAME:
        filename.append(dpslr::helpers::toLower(this->header_.basicInfo1Header()->target_name));
        break;
    }

    filename.append("_cpf_");

    // Append the starting date of the pass from H4.
    std::time_t time = dpslr::common::HRTimePoint::clock::to_time_t(this->header_.basicInfo2Header()->start_time);
    std::tm* tm = gmtime(&time);
    char start_date[7];
    std::strftime(start_date, 7, "%y%m%d", tm);
    filename.append(start_date);

    // Append the cpf version and source as file format.
    std::stringstream rr;

    if(this->header_.basicInfo1Header()->cpf_version >= 1 && this->header_.basicInfo1Header()->cpf_version < 2)
    {
        rr << std::setw(4) << std::setfill('0') << this->header_.basicInfo1Header()->cpf_sequence_number;
    }
    else if(this->header_.basicInfo1Header()->cpf_version >= 2 && this->header_.basicInfo1Header()->cpf_version < 3)
    {
        rr << std::setw(3) << std::setfill('0') << this->header_.basicInfo1Header()->cpf_sequence_number;
        rr << std::setw(2) << this->header_.basicInfo1Header()->cpf_subsequence_number;
    }

    filename.append('_' + rr.str() + '.' + this->header_.basicInfo1Header()->cpf_source);

    // Return the convention filename.
    return filename;
}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF OBSERVERS ---------------------------------------------------------------------------------------------------
bool CPF::empty() const {return this->empty_;}
// ---------------------------------------------------------------------------------------------------------------------

// --- CPF FILE METHODS ------------------------------------------------------------------------------------------------
CPF::ReadFileErrorEnum CPF::openCPFFile(const std::string &cpf_filepath, CPF::OpenOptionEnum open_option)
{
    // Variables.
    ReadFileErrorEnum error = ReadFileErrorEnum::NOT_ERROR;
    dpslr::common::RecordLinesVector data_vector;
    dpslr::common::RecordLinesVector cfg_vector;
    dpslr::common::RecordLinesVector header_vector;
    float version = 1.;
    bool header_finished = false;
    bool data_finished = false;
    bool read_finished = false;

    // Open the file using our custom input file stream.
    dpslr::helpers::InputFileStream cpf_stream(cpf_filepath);

    // Clear the CPF.
    this->clearCPF();

    // Check if the stream is open.
    if(!cpf_stream.is_open())
    {
        this->last_read_error_ = ReadFileErrorEnum::FILE_NOT_FOUND;
        return ReadFileErrorEnum::FILE_NOT_FOUND;
    }

    // Check if the stream is empty.
    if(cpf_stream.isEmpty())
    {
        this->last_read_error_ = ReadFileErrorEnum::FILE_EMPTY;
        return ReadFileErrorEnum::FILE_EMPTY;
    }

    // Store the file path and name.
    this->cpf_fullpath_ = cpf_filepath;
    this->cpf_filename_ = dpslr::helpers::split<std::vector<std::string>>(cpf_filepath, "/").back();

    // Open the header.
    while (!read_finished)
    {
        // Auxiliar variables.
        dpslr::common::ConsolidatedRecordStruct record;
        CPF::ReadRecordResultEnum read_result;

        // Get the next record.
        read_result = this->readRecord(cpf_stream, record);

        // Get the type.
        CPFRecordsTypeEnum type = static_cast<CPFRecordsTypeEnum>(record.generic_record_type);

        // Check the errors.
        if(read_result == ReadRecordResultEnum::UNDEFINED_RECORD)
        {
            this->clearCPFContents();
            this->last_read_error_ = ReadFileErrorEnum::UNDEFINED_RECORD;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::UNDEFINED_RECORD;
        }
        else if(type == CPFRecordsTypeEnum::HEADER_RECORD && header_finished)
        {
            this->clearCPFContents();
            this->last_read_error_ = ReadFileErrorEnum::ORDER_ERROR;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::ORDER_ERROR;
        }
        else if(type == CPFRecordsTypeEnum::DATA_RECORD && (!header_finished || data_finished))
        {
            this->clearCPFContents();
            this->last_read_error_ = ReadFileErrorEnum::ORDER_ERROR;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::ORDER_ERROR;
        }
        else if(type == CPFRecordsTypeEnum::EOH_RECORD && header_finished)
        {
            this->clearCPFContents();
            this->last_read_error_ = ReadFileErrorEnum::MULTIPLE_EOH;
            this->last_error_record_ = record;
            return ReadFileErrorEnum::MULTIPLE_EOH;
        }


        // Check the record type.
        if(type == CPFRecordsTypeEnum::HEADER_RECORD)
        {
            header_vector.push_back(record);
        }
        else if (type == CPFRecordsTypeEnum::EOH_RECORD)
        {
            // Check if we hava header records.
            if(!header_vector.empty())
            {
                // Save the possible issues.
                this->read_header_errors_ = this->header_.readHeader(header_vector);
                header_finished = true;

                // Get the version:
                if(this->header_.basicInfo1Header())
                    version = this->header_.basicInfo1Header()->cpf_version;
                else
                {
                    this->clearCPFContents();
                    this->last_read_error_ = ReadFileErrorEnum::VERSION_UNKNOWN;
                    return ReadFileErrorEnum::VERSION_UNKNOWN;
                }
            }
            else
            {
                this->clearCPFContents();
                this->last_read_error_ = ReadFileErrorEnum::NO_HEADER_FOUND;
                return ReadFileErrorEnum::NO_HEADER_FOUND;
            }
        }
        else if(type == CPFRecordsTypeEnum::DATA_RECORD)
        {
            // Store the data record.
            data_vector.push_back(record);
        }
        else if(type == CPFRecordsTypeEnum::EOE_RECORD)
        {
            // Check if we hava header records.
            if(data_vector.empty())
            {
                this->clearCPFContents();
                this->last_read_error_ = ReadFileErrorEnum::NO_DATA_FOUND;
                return ReadFileErrorEnum::NO_DATA_FOUND;
            }
            else if(!header_finished)
            {
                this->clearCPFContents();
                this->last_read_error_ = ReadFileErrorEnum::FILE_TRUNCATED;
                return ReadFileErrorEnum::FILE_TRUNCATED;
            }
            else
            {
                // Save the possible issues.
                this->read_data_errors_ = this->data_.readData(data_vector, version);
                data_finished = true;
            }

        }

        // Update the read finished variable.
        switch (open_option)
        {
            // Update the finished flag.
            case OpenOptionEnum::ONLY_HEADER:
            read_finished = header_finished;
            break;
            case OpenOptionEnum::ALL_DATA:
            read_finished = (header_finished && data_finished);
            break;
        }

        // Update the finished variable if the stream is empty.
        read_finished =  read_finished || cpf_stream.isEmpty();
    }

    // Check if the stream is not empty.
    if(data_finished && open_option == OpenOptionEnum::ALL_DATA && !cpf_stream.isEmpty())
    {
        //Clear.
        this->clearCPFContents();

        // Get the next line for error storing.
        std::string line;
        std::vector<std::string> tokens;
        cpf_stream.getline(line);
        dpslr::common::ConsolidatedRecordStruct rec;
        rec.line_number = cpf_stream.getLineNumber();
        rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::UNKNOWN_TYPE;
        if(!line.empty())
        {
            dpslr::helpers::split(tokens, line, " ", false);
            rec.tokens = tokens;
        }

        // Store the last error struct.
        this->last_error_record_ = rec;

        error = ReadFileErrorEnum::CONTENT_AFTER_EOE;
    }
    // Check if the file is truncated.
    else if(!header_finished)
    {
        error = ReadFileErrorEnum::FILE_TRUNCATED;
    }
    else if(!data_finished && open_option == OpenOptionEnum::ALL_DATA)
    {
        error = ReadFileErrorEnum::EOE_NOT_FOUND;
    }
    // Check if we have issues with the internal stored data.
    else if(this->read_header_errors_.empty() && this->read_data_errors_.empty())
    {
        error = ReadFileErrorEnum::NOT_ERROR;
    }
    else if(!this->read_header_errors_.empty() && this->read_data_errors_.empty())
    {
        error = ReadFileErrorEnum::HEADER_LOAD_WARNING;
    }
    else if(this->read_header_errors_.empty() && !this->read_data_errors_.empty())
    {
        error = ReadFileErrorEnum::DATA_LOAD_WARNING;
    }
    else
    {
        error = ReadFileErrorEnum::RECORDS_LOAD_WARNING;
    }

    // Clear the CPF if neccesary.
    if(error > ReadFileErrorEnum::DATA_LOAD_WARNING)
        this->clearCPFContents();
    else
        this->empty_ = false;

    this->last_read_error_ = error;

    // Return the error.
    return error;
}

CPF::ReadFileErrorEnum CPF::openCPFData()
{
    // Send path as copy to prevent clearing the path
    return this->openCPFFile(std::string(this->cpf_fullpath_), CPF::OpenOptionEnum::ALL_DATA);
}

CPF::WriteFileErrorEnum CPF::writeCPFFile(const std::string &cpf_filepath, bool force)
{
    std::ifstream input_file(cpf_filepath);
    if (input_file.good() && !force)
        return CPF::WriteFileErrorEnum::FILE_ALREADY_EXIST;

    if (!this->header_.basicInfo1Header())
        return CPF::WriteFileErrorEnum::VERSION_UNKNOWN;

    // Close the previous.
    input_file.close();

    // Open the file.
    std::ofstream output_file(cpf_filepath);

    // Store the records.
    output_file << this->header_.generateHeaderLines() << std::endl
                << this->EndRecordsString[static_cast<int>(CPFRecordsTypeEnum::EOH_RECORD)] << std::endl
                << this->data_.generateDataLines(this->header_.basicInfo1Header()->cpf_version) << std::endl
                << this->EndRecordsString[static_cast<int>(CPFRecordsTypeEnum::EOE_RECORD)];

    // Close the file.
    output_file.close();

    // Return no error.
    return CPF::WriteFileErrorEnum::NOT_ERROR;
}

CPF::ReadRecordResultEnum CPF::readRecord(dpslr::helpers::InputFileStream& stream,
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

            // Check the EOH case.
            if(tokens[0] == EndRecordsString[static_cast<int>(CPFRecordsTypeEnum::EOH_RECORD)])
            {
                rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CPF_TYPE;
                rec.tokens = tokens;
                rec.generic_record_type = static_cast<int>(CPFRecordsTypeEnum::EOH_RECORD);
                record_finished = true;
            }
            // Check the EOE case.
            else if(tokens[0] == EndRecordsString[static_cast<int>(CPFRecordsTypeEnum::EOE_RECORD)])
            {
                rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CPF_TYPE;
                rec.tokens = tokens;
                rec.generic_record_type = static_cast<int>(CPFRecordsTypeEnum::EOE_RECORD);
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
                if(dpslr::helpers::contains(CPFHeader::HeaderLineString, tokens[0]))
                {
                    rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CPF_TYPE;
                    rec.tokens = tokens;
                    rec.generic_record_type = static_cast<int>(CPFRecordsTypeEnum::HEADER_RECORD);
                    record_finished = true;
                }
                else if(dpslr::helpers::contains(CPFData::DataLineString, tokens[0]))
                {
                    rec.consolidated_type = dpslr::common::ConsolidatedFileTypeEnum::CPF_TYPE;
                    rec.tokens = tokens;
                    rec.line_number = stream.getLineNumber();
                    rec.generic_record_type = static_cast<int>(CPFRecordsTypeEnum::DATA_RECORD);
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
