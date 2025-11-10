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
 * @file class_cpf.h
 *
 * @brief This file contains an abstraction of Consolidated Laser Target Prediction Format (CPF),
 * for version 1 and 2, currently.
 *
 * This library is an abstraction of Consolidated Laser Target Prediction Format (CPF), for version 1 and 2, currently.
 * This class permits opening an existing CPF file and editing, or creating new files with raw data.
 *
 * This implementation also permits some less restrictive functionalities as convenient file naming generation using
 * NORAD, mixed files generation, and others.
 *
 * Also, the library contains methods for checking the consistency and integrity of all the introduced data [TODO],
 * and for including comment records (lines "00") when you need for clarity or to indicate important things.
 *
 * The "CPF Version 1" specification can be found in: https://ilrs.gsfc.nasa.gov/docs/2006/cpf_1.01.pdf
 * The "CPF Version 2" specification can be found in: https://ilrs.gsfc.nasa.gov/docs/2018/cpf_2.00h-1.pdf
 *
 * @author    Degoras Project Team.
 * @version   2202.1
 * @date      03-02-2022
 * @copyright EUPL License.
 *
 * @par IMPORTANT NOTE:
 *
 * This CPF code is provided "as is" with no guarantees. The software has been tested with a limited amount of data,
 * and there may still be errors and incomplete implementation of standards.
 *
 **********************************************************************************************************************/
#pragma once

// TODO: start datetime is midnight, but there are some ephemeris before? See format v2 remark
#include "libdpslr_global.h"
#include "common.h"
#include "helpers.h"

#include <array>

// ========== CPF HEADER ===============================================================================================
/**
 * @brief CPFHeader class manages header records.
 */
class LIBDPSLR_EXPORT CPFHeader
{
public:
    // CPF HEADERS LINES
    // -----------------------------------------------------------------------------------------------------------------
    static const std::array<std::string, 5> HeaderLineString;   ///< String header records identifiers.
    static const std::array<int, 2> CPFVersions;                ///< Add new main versions here.
    // -----------------------------------------------------------------------------------------------------------------

    // CPF HEADERS ENUMS
    // -----------------------------------------------------------------------------------------------------------------

    /// @enum HeaderRecordEnum
    /// This enum represents the different types of Header Records.
    enum class HeaderRecordEnum
    {
        BASIC_INFO_1_HEADER,         ///< Basic iformation 1 Header. Line H1 (mandatory).
        BASIC_INFO_2_HEADER,         ///< Basic information 2 Header. Line H2 (mandatory).
        EXPECTED_ACCURACY_HEADER,    ///< Expected accuracy Header. Line H3 (optional).
        TRANSPONDER_INFO_HEADER,     ///< Transponder information Header. Line H4 (optional).
        COM_CORRECTION_HEADER,       ///< Spherical satellite center of mass correction Header. Line H5 (optional).
    };

    /// @enum TargetClassEnum
    /// This enum represents the different classes of targets.
    enum class TargetClassEnum
    {
        NO_LRR,              ///< No retroreflector (including debris).                                    [only for v2]
        PASSIVE_LRR,         ///< Passive retroreflector.
        PASSIVE_LRR_LUNAR,   ///< Passive lunar retroreflector. In v2, use instead PASSIVE_LRR.       [deprecated in v2]
        SYNC_TRANSPONDER,    ///< Synchronous transponder.
        ASYNC_TRANSPONDER,   ///< Asynchronous transponder.
    };

    /// @enum ReferenceFrameEnum
    /// This enum represents the reference frame used for the coordinates system.
    enum class ReferenceFrameEnum
    {
        GEOCENTRIC_BODY_FIXED,             ///< Geocentric true body-fixed (default).
        GEOCENTRIC_SPACE_FIXED_TOD,        ///< Geocentric space-fixed (i.e, Inertial) (True-of-Date).
        GEOCENTRIC_SPACE_FIXED_MODJ2000,   ///< Geocentric space-fixed (Mean-of-Date J2000).
    };

    /// @enum RotAngleTypeEnum
    /// This enum represents the reference frame used for the coordinates system.
    enum class RotAngleTypeEnum
    {
        NOT_APPLICABLE,            ///< Geocentric true body-fixed (default).
        LUNAR_EULER_ANGLES,        ///< Lunar Euler angles: (phi, theta, psi).
        RA_DEC,                    ///< North pole Right Ascension and Declination, and angle to prime meridian.
    };

    /// @enum TargetDynamicsEnum
    /// This enum represents the target location/dynamics
    enum class TargetDynamicsEnum
    {
        OTHER,                  ///< Other.
        EARTH_ORBIT,            ///< Earth orbit.
        LUNAR_ORBIT,            ///< Lunar orbit.
        LUNAR_SURFACE,          ///< Lunar surface.
        MARS_ORBIT,             ///< Mars orbit.
        MARS_SURFACE,           ///< Mars surface.
        VENUS_ORBIT,            ///< Venus orbit.
        MERCURY_ORBIT,          ///< Mercury orbit.
        ASTEROID_ORBIT,         ///< Asteroid orbit.
        ASTEROID_SURFACE,       ///< Asteroid surface.
        SOLAR_TRANSFER_ORBIT    ///< Solar orbit/transfer orbit (includes fly-by).
    };

    // -----------------------------------------------------------------------------------------------------------------

    // CPF HEADERS STRUCTS
    // -----------------------------------------------------------------------------------------------------------------
    /**
     * @brief H1 - Basic Information 1 Header (required).
     */
    struct BasicInfo1Header : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.
        float cpf_version;                              ///< CPF version. We store the subversion, for example 2.1
        std::string cpf_source;                         ///< Ephemeris source.
        dpslr::common::HRTimePoint cpf_production_date;   ///< File production date. Will update if we call generate line.
        std::string target_name;                        ///< Target name from official ILRS list.
        std::string cpf_notes;                          ///< Notes with NO SPACES.
        int cpf_sequence_number;                        ///< Ephemeris sequence number.
        int cpf_subsequence_number;                     ///< Ephemeris sub-daily sequence number.               [For v2]
        // Functions.
        /**
         * @brief Generate the line for this record. Also regenerate the cpf_production_date.
         * @return The generated line
         * @todo
         */
        std::string generateLine();
    };

    /**
     * @brief H2 - Basic Informat Header (required).
     */
    struct BasicInfo2Header : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.
        std::string id;                                 ///< It is always ILRS ID, based on COSPAR ID.
        dpslr::common::optional<std::string> sic;       ///< SIC provided by ILRS. Set to -1 if target has no SIC.
        std::string norad;                              ///< NORAD ID.
        dpslr::common::HRTimePoint start_time;            ///< Ephemeris start time.
        dpslr::common::HRTimePoint end_time;              ///< Ephemeris end time.
        std::chrono::seconds total_seconds;             ///< TODO: what is this? non-standard.
        std::chrono::seconds time_between_entries;      ///< Time between two table entries in seconds. 0 if variable.
        bool tiv_compatible;                            ///< Compatible with TIVs.
        TargetClassEnum target_class;                   ///< Target class.
        ReferenceFrameEnum reference_frame;             ///< Reference frame.
        RotAngleTypeEnum rot_angle_type;                ///< Rotation angles type.
        bool com_applied;                               ///< Center of mass correction. True applied, false not applied.
        TargetDynamicsEnum target_dynamics;             ///< Target location/dynamics.                     [only for v2]

        // Functions.
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief TODO: H3 - Expected accuracy (optional).
     * @todo
     */
    struct ExpectedAccuracyHeader : dpslr::common::ConsolidatedRecordStruct
    {
        // Members

        // Functions.
        /**
         * @brief TODO: Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief TODO: H4 - Transponder information (optional).
     * @todo
     */
    struct TransponderInfoHeader : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.

        // Functions.
        /**
         * @brief TODO: Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief H5 - Spherical satellite center of mass correction Header (optional).
     */
    struct CoMCorrectionHeader : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.
        double com_correction;                           ///< Approx. center of mass to reflector offset in m (positive)
        // Functions.
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };
    // -----------------------------------------------------------------------------------------------------------------

    // Constructor.
    /**
     * @brief CPFHeader constructor filling specific version.
     * @param cpf_version
     */
    explicit CPFHeader(float cpf_version);

    /**
     * @brief CPFHeader default constructor. Version is not filled.
     */
    explicit CPFHeader() = default;

    // Copy and movement constructors and assign operators
    CPFHeader(const CPFHeader&) = default;
    CPFHeader(CPFHeader&&) = default;
    CPFHeader& operator = (const CPFHeader&) = default;
    CPFHeader& operator = (CPFHeader&&) = default;

    // Destructor.
    ~CPFHeader() = default;

    // Clear methods.
    /**
     * @brief Clears all registers.
     */
    void clearAll();
    /**
     * @brief Clears the Basic Info 1 Header.
     */
    void clearBasicInfo1Header();
    /**
     * @brief Clears the Basic Info 2 Header.
     */
    void clearBasicInfo2Header();
    /**
     * @brief Clears the Expected Accuracy Header.
     */
    void clearExpectedAccuracyHeader();
    /**
     * @brief Clears the Transponder Information Header.
     */
    void clearTransponderInfoHeader();
    /**
     * @brief Clears the Center of Mass Correction Header.
     */
    void clearCoMCorrectionHeader();

    // Headers getters.
    const dpslr::common::optional<BasicInfo1Header> &basicInfo1Header() const;
    const dpslr::common::optional<BasicInfo2Header> &basicInfo2Header() const;
    const dpslr::common::optional<ExpectedAccuracyHeader> &expectedAccuracyHeader() const;
    const dpslr::common::optional<TransponderInfoHeader> &transponderInfoHeader() const;
    const dpslr::common::optional<CoMCorrectionHeader> &coMCorrectionHeader() const;

    // Headers getters non const.
    dpslr::common::optional<BasicInfo1Header> &basicInfo1Header();
    dpslr::common::optional<BasicInfo2Header> &basicInfo2Header();
    dpslr::common::optional<ExpectedAccuracyHeader> &expectedAccuracyHeader();
    dpslr::common::optional<TransponderInfoHeader> &transponderInfoHeader();
    dpslr::common::optional<CoMCorrectionHeader> &coMCorrectionHeader();

    // Headers setters.
    void setBasicInfo1Header(const BasicInfo1Header&);
    void setBasicInfo2Header(const BasicInfo2Header&);
    void setExpectedAccuracyHeader(const ExpectedAccuracyHeader&);
    void setTransponderInfoHeader(const TransponderInfoHeader&);
    void setCoMCorrection(const CoMCorrectionHeader&);

    /**
     * @brief Generate CPF header lines.
     * @return The generated lines.
     */
    std::string generateHeaderLines();

    // Generic read methods.
    /**
     * @brief Read header records.
     * @param rec_v, vector with header records.
     * @return The errors generated at reading.
     */
    dpslr::common::RecordReadErrorMultimap readHeader(const dpslr::common::RecordLinesVector &rec_v);

    // Specific read methods.
    /**
     * @brief Read Basic Info 1 Header H1 from record.
     * @param rec, the record to read from.
     * @return The error generated at reading.
     */
    dpslr::common::RecordReadErrorEnum readBasicInfo1Header(const dpslr::common::ConsolidatedRecordStruct &rec);

    /**
     * @brief Read Basic Info 2 Header H2 from record.
     * @param rec, the record to read from.
     * @return The error generated at reading.
     */
    dpslr::common::RecordReadErrorEnum readBasicInfo2Header(const dpslr::common::ConsolidatedRecordStruct &rec);

    /**
     * @brief Read Expected Accuracy H3 from record.
     * @param rec, the record to read from.
     * @return The error generated at reading.
     */
    dpslr::common::RecordReadErrorEnum readExpectedAccuracyHeader(const dpslr::common::ConsolidatedRecordStruct &rec);

    /**
     * @brief Read Transponder Info H4 from record.
     * @param rec, the record to read from.
     * @return The error generated at reading.
     */
    dpslr::common::RecordReadErrorEnum readTransponderInfoHeader(const dpslr::common::ConsolidatedRecordStruct &rec);

    /**
     * @brief Read Center of Mass Correction H5 from record.
     * @param rec, the record to read from.
     * @return The error generated at reading.
     */
    dpslr::common::RecordReadErrorEnum readCoMCorrectionHeader(const dpslr::common::ConsolidatedRecordStruct &rec);


private:

    // Generic private read method.
    dpslr::common::RecordReadErrorEnum readHeaderLine(const dpslr::common::RecordLinePair &lpair);

    // H1 (required), H2 (required), H3, H4, H5
    dpslr::common::optional<BasicInfo1Header> basic_info1_header;
    dpslr::common::optional<BasicInfo2Header> basic_info2_header;
    dpslr::common::optional<ExpectedAccuracyHeader> exp_accuracy_header;
    dpslr::common::optional<TransponderInfoHeader> transp_info_header;
    dpslr::common::optional<CoMCorrectionHeader> com_corr_header;
};
// =====================================================================================================================


// ========== CPF Data =================================================================================================
class LIBDPSLR_EXPORT CPFData
{
public:

    // CPF DATA LINES
    // -----------------------------------------------------------------------------------------------------------------
    static const std::array<const char*, 7> DataLineString;            ///< String data records identifiers.
    // -----------------------------------------------------------------------------------------------------------------

    // CPF DATA ENUMS
    // -----------------------------------------------------------------------------------------------------------------
    /// @enum DataRecordTypeEnum
    /// @brief Data record types.
    enum class DataRecordTypeEnum
    {
        POSITION_RECORD               = 0,     ///< Position record.
        VELOCITY_RECORD               = 1,     ///< Velocity record.
        CORRECTIONS_RECORD            = 2,     ///< Corrections record.
        TRANSPONDER_RECORD            = 3,     ///< Transponder specific record.
        OFFSET_FROM_CENTER_RECORD     = 4,     ///< Offset from center of main body record.
        ROT_ANGLE_RECORD              = 5,     ///< Rotation angle of offset record.
        EARTH_ORIENTATION_RECORD      = 6      ///< Earth orientation record.
    };

    /// @enum DirectionFlagEnum
    /// @brief Direction flag.
    enum class DirectionFlagEnum
    {
        COMMON_EPOCH  = 0,           ///< Common epoch. Instantaneous vector between geocenter and target.
        TRANSMIT      = 1,           ///< Transmit. Iterated travel time from geocenter to target at transmit epoch.
        RECEIVE       = 2            ///< Receive. Iterated travel time from target to geocenter at receive epoch.
    };

    // -----------------------------------------------------------------------------------------------------------------

    // CPF DATA STRUCTS
    // -----------------------------------------------------------------------------------------------------------------
    /**
     * @brief 10 - POSITION RECORD
     */
    struct PositionRecord : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.
        DirectionFlagEnum dir_flag;                         ///< Direction flag.
        int mjd;                                            ///< Modified Julian Date.
        long double sod;                                    ///< Second of day (UTC)
        int leap_second;                                    ///< Leap second flag (0 or the value of new leap second)
        std::array<long double, 3> geocentric_pos;          ///< Geocentric position in meters (x, y, z)

        // Functions.
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief 20 - VELOCITY RECORD
     */
    struct VelocityRecord : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.
        DirectionFlagEnum dir_flag;                         ///< Direction flag
        std::array<long double, 3> geocentric_vel;          ///< Geocentric velocity in m/s (x, y, z)

        // Functions.
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief 30 - CORRECTIONS RECORD
     */
    struct CorrectionsRecord : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.
        DirectionFlagEnum dir_flag;                         ///< Direction flag
        std::array<long double, 3> aberration_correction;   ///< Stellar aberration correction in meters (x, y, z)
        double range_correction;                            ///< Relativistic range correction in ns (positive)
        // Functions.
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief TODO: 40 - TRANSPONDER RECORD
     * @todo
     */
    struct TransponderRecord : dpslr::common::ConsolidatedRecordStruct
    {
        // Members.

        // Functions.
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief 50 - OFFSET FROM CENTER OF MAIN BODY RECORD
     * @todo
     */
    struct OffsetFromCenterRecord : dpslr::common::ConsolidatedRecordStruct
    {
        // Members

        // Functions
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief TODO: 60 - ROTATION ANGLE OF OFFSET RECORD
     */
    struct RotationAngleRecord : dpslr::common::ConsolidatedRecordStruct
    {
        // Members

        // Functions
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    /**
     * @brief TODO: 70 - EARTH ORIENTATION RECORD
     */
    struct EarthOrientationRecord : dpslr::common::ConsolidatedRecordStruct
    {
        // Members

        // Functions
        /**
         * @brief Generate the line for this record.
         * @param version
         * @return The generated line.
         * @todo
         */
        std::string generateLine(float version) const;
    };

    // Constructor.
    CPFData() = default;

    // Copy and movement constructors and assign operators
    CPFData(const CPFData&) = default;
    CPFData(CPFData&&) = default;
    CPFData& operator = (const CPFData&) = default;
    CPFData& operator = (CPFData&&) = default;

    // Destructor.
    ~CPFData() = default;

    // Clear methods.
    /**
     * @brief Clear all records.
     */
    void clearAll();
    /**
     * @brief Clears all the position records
     */
    void clearPositionRecords();
    /**
     * @brief Clears all the Position Records
     */
    void clearVelocityRecords();
    /**
     * @brief Clears all the Velocity Records
     */
    void clearCorrectionRecords();
    /**
     * @brief Clears all the Correction Records
     */
    void clearTransponderRecords();
    /**
     * @brief Clears all the Transponder Records
     */
    void clearOffsetFromCenterRecords();
    /**
     * @brief Clears all the Offset From Center Records
     */
    void clearRotationAngleRecords();
    /**
     * @brief Clears all the Earth Orientation Records
     */
    void clearEarthOrientationRecords();

    // Const getters
    const std::vector<PositionRecord> &positionRecords() const;
    const std::vector<VelocityRecord> &velocityRecords() const;
    const std::vector<CorrectionsRecord> &correctionsRecords() const;
    const std::vector<TransponderRecord> &transponderRecords() const;
    const std::vector<OffsetFromCenterRecord> &offsetFromCenterRecords() const;
    const std::vector<RotationAngleRecord> &rotationAngleRecords() const;
    const std::vector<EarthOrientationRecord> &earthOrientationRecords() const;

    // Non-const getters
    std::vector<PositionRecord> &positionRecords();
    std::vector<VelocityRecord> &velocityRecords();
    std::vector<CorrectionsRecord> &correctionsRecords();
    std::vector<TransponderRecord> &transponderRecords();
    std::vector<OffsetFromCenterRecord> &offsetFromCenterRecords();
    std::vector<RotationAngleRecord> &rotationAngleRecords();
    std::vector<EarthOrientationRecord> &earthOrientationRecords();


    // Setter methods
    void setPositionRecords(const std::vector<PositionRecord>&);
    void setVelocityRecords(const std::vector<VelocityRecord>&);
    void setCorrectionsRecords(const std::vector<CorrectionsRecord>&);
    void setTransponderRecords(const std::vector<TransponderRecord>&);
    void setOffsetFromCenterRecords(const std::vector<OffsetFromCenterRecord>&);
    void setRotationAngleRecords(const std::vector<RotationAngleRecord>&);
    void setEarthOrientationRecords(const std::vector<EarthOrientationRecord>&);


    // Records individual setter methods.
    /**
     * @brief Adds a position record to the end of the current list of position records.
     * @param rec, the position record to append.
     */
    void addPositionRecord(const PositionRecord &rec);
    /**
     * @brief Adds a velocity record to the end of the current list of velocity records.
     * @param rec, the velocity record to append.
     */
    void addVelocityRecord(const VelocityRecord &rec);
    /**
     * @brief Adds a corrections record to the end of the current list of corrections records.
     * @param rec, the corretions record to append.
     */
    void addCorrectionsRecord(const CorrectionsRecord &rec);
    /**
     * @brief Adds a transponder record to the end of the current list of transponder records.
     * @param rec, the transponder record to append.
     */
    void addTransponderRecord(const TransponderRecord &rec);
    /**
     * @brief Adds an offset from center record to the end of the current list of offset from center records.
     * @param rec, the offset from center record to append.
     */
    void addOffsetFromCenterRecord(const OffsetFromCenterRecord &rec);
    /**
     * @brief Adds a rotation angle record to the end of the current list of rotation angle records.
     * @param rec, the rotation angle record to append.
     */
    void addRotationAngleRecord(const RotationAngleRecord &rec);
    /**
     * @brief Adds an earth orientation record to the end of the current list of earth orientation records.
     * @param rec, the earth orientation record to append.
     */
    void addEarthOrientationRecord(const EarthOrientationRecord &rec);

    // Generate CPF lines
    /**
     * @brief Generates all CPF data lines.
     * @param version
     * @return The CPF data lines generated.
     * @todo
     */
    std::string generateDataLines(float version) const;

    // Generate specific CPF lines.
    /**
     * @brief Generates position records (10) lines.
     * @param version
     * @return The CPF position records generated lines.
     * @todo
     */
    std::string generatePositionRecordsLines(float version) const;
    /**
     * @brief Generates velocity records (20) lines.
     * @param version
     * @return The CPF velocity records generated lines.
     * @todo
     */
    std::string generateVelocityRecordsLines(float version) const;
    /**
     * @brief Generates corrections records (30) lines.
     * @param version
     * @return The CPF corrections records generated lines.
     * @todo
     */
    std::string generateCorrectionsRecordsLines(float version) const;
    /**
     * @brief Generates transponder records (40) lines.
     * @param version
     * @return The CPF transponder records generated lines.
     * @todo
     */
    std::string generateTransponderRecordsLines(float version) const;
    /**
     * @brief Generates offset from center records (50) lines.
     * @param version
     * @return The CPF offset from center records generated lines.
     * @todo
     */
    std::string generateOffsetFromCenterRecordsLine(float version) const;
    /**
     * @brief Generates rotation angle records (60) lines.
     * @param version
     * @return The CPF rotation angle records generated lines.
     * @todo
     */
    std::string generateRotationAngleRecordsLines(float version) const;
    /**
     * @brief Generates earth orientation records (70) lines.
     * @param version
     * @return The CPF earth orientation records generated lines.
     * @todo
     */
    std::string generateEarthOrientationRecordsLine(float version) const;

    // Generic read methods.
    // Calls to clearAll()
    dpslr::common::RecordReadErrorMultimap readData(const dpslr::common::RecordLinesVector &rec_v, float version);

    // Specific read methods.
    // The read data-line functions append the data to the proper vector. Do nothing if error.
    // The read data functions clear the pointers (struct) and create a new one (or nullptr if error).
    dpslr::common::RecordReadErrorEnum readPositionRecord(const dpslr::common::ConsolidatedRecordStruct&, float);
    dpslr::common::RecordReadErrorEnum readVelocityRecord(const dpslr::common::ConsolidatedRecordStruct&, float);
    dpslr::common::RecordReadErrorEnum readCorrectionsRecord(const dpslr::common::ConsolidatedRecordStruct&, float);
    dpslr::common::RecordReadErrorEnum readTransponderRecord(const dpslr::common::ConsolidatedRecordStruct&, float);
    dpslr::common::RecordReadErrorEnum readOffsetFromCenterRecord(const dpslr::common::ConsolidatedRecordStruct&, float);
    dpslr::common::RecordReadErrorEnum readRotationAngleRecord(const dpslr::common::ConsolidatedRecordStruct&, float);
    dpslr::common::RecordReadErrorEnum readEarthOrientationRecord(const dpslr::common::ConsolidatedRecordStruct&, float);

    // TODO:
    // Integrity Checker.
    // Sorting the records or ensure order

private:

    // Generic private read method.
    dpslr::common::RecordReadErrorEnum readDataLine(const dpslr::common::RecordLinePair &rpair, float version);

    // Private vectors for store the different data records.
    std::vector<PositionRecord> pos_records;
    std::vector<VelocityRecord> vel_records;
    std::vector<CorrectionsRecord> corr_records;
    std::vector<TransponderRecord> transp_records;
    std::vector<OffsetFromCenterRecord> offset_records;
    std::vector<RotationAngleRecord> rot_angle_records;
    std::vector<EarthOrientationRecord> earth_orientation_records;

};

// =====================================================================================================================

class LIBDPSLR_EXPORT CPF
{
public:

    static const std::array<const char*, 2> EndRecordsString;

    // CPF DATA ENUMS
    // -----------------------------------------------------------------------------------------------------------------

    /// @enum CPFRecordsTypeEnum
    /// This enum represents the differents CPF record types.
    enum class CPFRecordsTypeEnum
    {
        EOH_RECORD,     ///< End of Header record (H9).
        EOE_RECORD,     ///< End of Ephemeris record (99).
        HEADER_RECORD,  ///< Header records (H1, H2, H3, H4, H5).
        DATA_RECORD     ///< Data records (10, 20, 30, 40, 50, 60, 70).
    };

    /// @enum OpenOptionEnum
    /// This enum represents the possible options for reading CPF files. Used in the function @ref openCPFFile.
    enum class OpenOptionEnum
    {
        ONLY_HEADER,    ///< Only header records will be read. Use this for faster processing (i.e. searching files).
        ALL_DATA        ///< All records will be read, including data records.
    };

    /// @enum TargetIdOptionEnum
    /// This enum represents the possible target identifiers that can be used for generating the CPF filename. It is
    /// used in the function @ref getStandardFilename.
    enum class TargetIdOptionEnum
    {
        TARGET_NAME,   ///< Target name (a very bad identifier for debris targets!).
        ILRS_ID,       ///< ILRS identifier (eg. 8606101).
        NORAD,         ///< NORAD identifier (eg. 16908 ).
        COSPAR,        ///< COSPAR identifier: 1986-061A
        SHORT_COSPAR   ///< SHORT COSPAR identifier: 86061A
    };

    /// @enum ReadFileErrorEnum
    /// This enum represents the posible errors at CPF file reading.
    enum class ReadFileErrorEnum
    {
        NOT_ERROR,            ///< No error.
        RECORDS_LOAD_WARNING, ///< Issues in multiple types of records. Use the getters for get specific ones.
        HEADER_LOAD_WARNING,  ///< Issues in header records. Use @ref getReadHeaderErrors for get specific ones.
        DATA_LOAD_WARNING,    ///< Issues in data records. Use @ref getReadDataErrors for get specific ones.
        UNDEFINED_RECORD,     ///< Read an undefined record. The wrong record can be get using @ref getLastErrorStruct.
        ORDER_ERROR,          ///< Bad record order in the file. The record can be get using @ref getLastErrorStruct.
        MULTIPLE_EOH,         ///< Multiple end of session (H9). The record can be get using @ref getLastErrorStruct.
        CONTENT_AFTER_EOE,    ///< Content after the end of file (99) record. 99 must be the last line always.
        EOH_NOT_FOUND,        ///< The end of session token (H9) is not found.
        EOE_NOT_FOUND,        ///< The end of file token (99) is not found.
        FILE_NOT_FOUND,       ///< The CPF file is not found.
        FILE_EMPTY,           ///< The CPF file is empty.
        FILE_TRUNCATED,       ///< The file is truncated. The end of file was reached before completing the data read.
        NO_HEADER_FOUND,      ///< No header records were found.
        NO_DATA_FOUND,        ///< No data records were found.
        VERSION_UNKNOWN       ///< Header records loaded, but version missing (maybe missing H1).
    };

    /// @enum WriteFileErrorEnum
    /// This enum represents the posible errors at CPF file writing.
    enum class WriteFileErrorEnum
    {
        NOT_ERROR,            ///< No error.
        FILE_ALREADY_EXIST,   ///< The file already exist and force option was not specified.
        VERSION_UNKNOWN       ///< Version missing (missing H1).
    };

    /// @enum ReadRecordResultEnum
    /// This enum represents the posible errors when reading each record.
    enum class ReadRecordResultEnum
    {
        NOT_ERROR,           ///< No error.
        STREAM_NOT_OPEN,     ///< The file is not found.
        STREAM_EMPTY,        ///< The file is empty.
        UNDEFINED_RECORD     ///< The readead record is not recognized.
    };


    // -----------------------------------------------------------------------------------------------------------------

    // Constructors.
    /**
     * @brief Empty constructor to create an empty CPF. It can be opened later with @ref openCPFFile (this function
     * will update the version). Otherwise, you can add/modify records using the getters to generate your custom CPF.
     * @param version
     */
    explicit CPF(float version = 2.0);

    /**
     * @brief Constructor to create a CPF by reading a file. Remember to check the possible errors.
     * @param cpf_filepath
     * @param open_data
     */
    explicit CPF(const std::string& cpf_filepath, OpenOptionEnum open_option);

    // Copy and movement constructors and assign operators
    CPF(const CPF&) = default;
    CPF(CPF&&) = default;
    CPF& operator = (const CPF&) = default;
    CPF& operator = (CPF&&) = default;

    // Clear methods.
    /**
     * @brief Clears all data in CPF, including data, header, error structs and filename.
     */
    void clearCPF();
    /**
     * @brief Clears data and header in CPF, leaving it empty. Error structs and filename are kept.
     */
    void clearCPFContents();
    /**
     * @brief Clears the CPF header.
     */
    void clearCPFHeader();
    /**
     * @brief Clears the CPF data.
     */
    void clearCPFData();


    // CPF header and data getter methods for records edition as modifiers.
    CPFHeader &getHeader();
    CPFData &getData();

    // CPF header, cfg and data const getter methods.
    const CPFHeader &getHeader() const;
    const CPFData &getData() const;

    // Error getters.
    const dpslr::common::RecordReadErrorMultimap& getReadHeaderErrors() const;
    const dpslr::common::RecordReadErrorMultimap& getReadDataErrors() const;
    ReadFileErrorEnum getLastReadError() const;
    const dpslr::common::optional<dpslr::common::ConsolidatedRecordStruct>& getLastReadErrorRecord() const;

    // Filename getters.
    const std::string& getSourceFilename() const;
    const std::string& getSourceFilepath() const;

    // Observers
    /**
     * @brief Checks if CPF is empty. A CPF will be empty if both header and data are empty. This can happen if an error
     * occurs at CPF reading, or if user explicitly clears data and header.
     * This method is only valid if CPF is used for reading. For writing, the user is responsible to track the emptiness
     * status of the CPF.
     * @return true if CPF is empty, false otherwise.
     */
    bool empty() const;

    // File Methods
    /**
     * @brief Custom getter that generates the filename using the CPF naming convention.
     *
     * This function generates a ILRS CPF standard convention filename. The convention is:
     * - For v1:
     *    targetname_cpf_yymmdd_nnnv.src (src is source, nnn sequence number and v sub-daily sequence number)
     *
     * - For v2:
     *    targetname_cpf_yymmdd_nnnvv.src (src is source, nnn sequence number and vv sub-daily sequence number)
     *
     * The targetname is the official name defined by ILRS. It can be changed using @p id_option
     *
     * @pre The Basic Info 1 Header (H1) must be defined.
     *      The Basic Info 2 Header (H2) must be defined.
     *
     * @param[in] id_option Determines which identifier will be used for the file name. Default is name.
     *
     * @return A string that contains the generated standard filename. If error, it returns an empty string.
     *
     * @note Changing the ID that appears in the file name is interesting, since different stations can use different
     *       target names, especially when tracking debris. Also, using other more standard IDs allows faster and
     *       simpler file search systems.
     */
    std::string getStandardFilename(TargetIdOptionEnum id_option = TargetIdOptionEnum::TARGET_NAME) const;

    /**
     * @brief Open a CPF file.
     *
     * This function opens an ILRS CPF file, and store the records in memory. Using the open
     * options, you can open the header records only, or all the records. This can be useful for fast opening while
     * searching certain files. This call doesn't modify the content of the file.
     *
     * @post This call will clear all the structs with the previous data, calling to clearCPF().
     *       This call will clear the read error vectors with the previous errors.
     *       This call will open the file, store the records in memory, and close the file.
     *
     * @param[in] cpf_filepath String with the complete path where the CPF file is stored.
     * @param[in] open_option Determines which structures will be read and stored.
     *
     * @return A @ref ReadFileErrorEnum value that contains the possible error.
     */
    ReadFileErrorEnum openCPFFile(const std::string& cpf_filepath, OpenOptionEnum open_option);

    /**
     * @brief Open data records of a previously opened CPF file.
     *
     * This function opens all records (including the data records) of an ILRS CPF file, and store
     * the records in memory. This function will call to @ref openCPFFile internally.
     *
     * @pre A CPF file MUST have been opened with @ref CPF(const std::string&, OpenOptionEnum) or the @ref openCPFFile,
     *      since this function will use the previously stored file path.
     *
     * @post This call will clear all the structs with the previous data, calling to clearCPF().
     *       This call will clear the read error vectors with the previous errors.
     *       This call will open the file, store the records in memory, and close the file.
     *
     * @return A @ref ReadFileErrorEnum value that contains the possible error.
     */
    ReadFileErrorEnum openCPFData();

    /**
     * @brief Writes a CPF file.
     *
     * This function writes the records stored in memory into an ILRS CPF file.
     *
     * @post
     *
     * @param[in] cpf_filepath String with the complete path where the CPF file will be written.
     * @param[in] force Boolean that indicates if the writing can be forced even if the file already exists.

     * @return A @ref WriteFileErrorEnum value that contains the possible error.
     */
    WriteFileErrorEnum writeCPFFile(const std::string& cpf_filepath, bool force = false);


    // Check integrity method.
    // TODO

private:

    // Private methods for reading records.
    // Return false if error.
    ReadRecordResultEnum readRecord(dpslr::helpers::InputFileStream&, dpslr::common::ConsolidatedRecordStruct&);

    // Empty.
    bool empty_;

    // Data, configuration and header storage.
    CPFHeader header_;
    CPFData data_;

    // Private members for store read errors.
    dpslr::common::RecordReadErrorMultimap read_header_errors_;
    dpslr::common::RecordReadErrorMultimap read_data_errors_;
    ReadFileErrorEnum last_read_error_;
    dpslr::common::optional<dpslr::common::ConsolidatedRecordStruct> last_error_record_;

    // File names and path.
    std::string cpf_filename_;
    std::string cpf_fullpath_;
};
// =====================================================================================================================


// ========== EXTERNAL OPERATORS =======================================================================================
//bool LIBDPSLR_EXPORT operator <(const CPF& a, const CPF& b);
//bool LIBDPSLR_EXPORT operator >(const CPF& a, const CPF& b);
// =====================================================================================================================

