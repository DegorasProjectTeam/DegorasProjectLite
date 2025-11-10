#pragma once

#include "class_calibration.h"
#include "class_salarainformation.h"
#include "spcore_global.h"

class QFile;

class SP_CORE_EXPORT CalibrationFileManager
{
public:
    enum ErrorEnum
    {
        CALIBFILE_NOT_OPEN,
        CALIBFILE_INVALID,
        CALIB_NOT_FOUND
    };

    static const QMap<ErrorEnum, QString> ErrorListStringMap;

    static SalaraInformation readCalibration(const QString& cal_name, const QString &dir_path, Calibration& calib);
    static SalaraInformation readCalibration(const QString& cal_name, Calibration& calib);
    static SalaraInformation readCalibrationDir(const QString& dir, std::vector<Calibration>& calibs);
    static SalaraInformation readLastCalib(Calibration &calib);
    static SalaraInformation writeCalibration(const Calibration& calib, const QString& dest_dir,
                                              const QString& dest_file = "" );
    static QString calibrationFilename(const Calibration &calib);
    static QString findCalibration(const QString &calib_name);

    static QDate startDate(const QString &calib_name);
    static QDateTime startDateTime(const QString &calib_name);

private:
    static SalaraInformation readCalibrationFromFile(const QString &filepath, Calibration& calib);
};

