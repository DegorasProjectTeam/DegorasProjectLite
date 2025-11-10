#pragma once

#include <QList>
#include <QStringList>
#include <QString>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>
#include <QMap>

#include <memory>

#include "class_spaceobject.h"
#include "class_spaceobjectmodel.h"
#include "class_salarainformation.h"
#include "spcore_global.h"

class SP_CORE_EXPORT SpaceObjectFileManager
{
public:

    enum ErrorEnum
    {
        DATAFILE_NOT_OPEN,
        SCHEMEFILE_NOT_OPEN,
        SETSFILE_NOT_OPEN,
        DATAFILE_INVALID,
        SCHEMEFILE_INVALID,
        INTEGRITY_ERROR,
        SCHEME_NOT_VALID
    };

    static const QMap<ErrorEnum, QString> ErrorListStringMap;

    // Static class. Delete constructor.
    SpaceObjectFileManager() = delete;

    // Load methods.
    static SalaraInformation
    loadSpaceObjectsDataAsObjectsList(const QString& path_data, const QString& path_scheme,
                                      SpaceObjectsList& list, QString& comment,
                                      QString& version, QDateTime& versiondate);

    static SalaraInformation
    loadSpaceObjectsDataAsObjects(const QString& path_data, const QString& path_scheme,
                                  SpaceObjectsMap& map, QString& comment,
                                  QString& version,  QDateTime& versiondate);
    static SalaraInformation
    loadSpaceObjectsDataAsModel(const QString& path_data, const QString& path_scheme, SpaceObjectModel& model,
                                QString& comment, QString& version, QDateTime& versiondate);

    static SalaraInformation loadSpaceObjectsSetsFile(const QString& path_set, QList<SpaceObjectSet*>& sets,
                                                      QString& current_set);

    // Save methods.
    static SalaraInformation saveSpaceObjectsData(const QString& path_data, const QString& version_name,const QString& version_date,
                                                  const QString& comment, const SpaceObjectsList& list);
    static SalaraInformation
    saveSpaceObjectsData(const QString& path_data, const QString& version_name, const QDateTime& version_date,
                          const QString& comment, const QJsonArray& list, const QJsonArray &extraparameters);

    static SalaraInformation saveSets(const QString& path_sets, const QDateTime& version_date, const QList<SpaceObjectSet*>& sets);

    static SalaraInformation saveCurrentSet(const QString &current_set, const QString &path_sets);
    static SalaraInformation getCurrentSet(const QString &path_sets, QString &current_set);
    static SalaraInformation loadSpaceObjectsScheme(const QString &path_scheme, SpaceObjectModel &model);
    static SalaraInformation loadSpaceObjectsData(const QString &path_data, SpaceObjectModel &model, QString& comment,
                                                  QString& version, QDateTime& versiondate);
};
