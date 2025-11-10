#pragma once

#include "class_tracking.h"
#include "class_salarainformation.h"
#include "spcore_global.h"

class SP_CORE_EXPORT TrackingFileManager
{
public:
    enum ErrorEnum
    {
        TRACKFILE_NOT_OPEN,
        TRACKFILE_INVALID,
        TRACKFILE_NOT_EXISTS,
        TRACKFILE_NOT_REMOVABLE
    };

    static const QMap<ErrorEnum, QString> ErrorListStringMap;

    static SalaraInformation readTracking(const QString& track_name, const QString &track_dirpath,
                                          const QString &calib_dirpath, Tracking& track);
    static SalaraInformation readTracking(const QString& track_name, const QString &track_dirpath, Tracking& track);
    static SalaraInformation readTracking(const QString& track_name, Tracking& track);
    static SalaraInformation readTrackingDir(const QString& dir, const QString& calib_path,
                                             std::vector<Tracking>& tracks);
    static SalaraInformation readTrackingDir(const QString& dir, std::vector<Tracking>& tracks);
    static SalaraInformation writeTracking(const Tracking& track, const QString& dest_dir = "",
                                           const QString& filename = "");
    static SalaraInformation removeTracking(const QString& track_name);
    static SalaraInformation removeCurrentTracking(const QString& track_name);

    static QString trackingFilename(const Tracking &track);
    static QString findTracking(const QString &track_name);
    static QStringList findTrackings(const QDateTime &start, const QDateTime &end, const QString &object_norad = "",
                                     const QString &cfg_id = "", const QString& dir = "");
    static QStringList currentTrackings(const QString &object_norad = "", const QString &cfg_id = "");

    static QDate startDate(const QString &track_name);

private:
    static SalaraInformation readTrackingFromFile(const QString &file_path, const QString &calib_path, Tracking& track);
    static SalaraInformation writeTrackingPrivate(const Tracking& track, const QString& filepath);
};

