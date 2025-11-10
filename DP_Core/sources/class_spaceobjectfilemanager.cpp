#include "includes/global_texts.h"
#include "includes/class_spaceobjectfilemanager.h"

#include <QJsonDocument>
#include <QDebug>
#include <QFile>


const QMap<SpaceObjectFileManager::ErrorEnum, QString> SpaceObjectFileManager::ErrorListStringMap =
{
    {SpaceObjectFileManager::ErrorEnum::DATAFILE_INVALID,
     "The json file '"+QString(FILE_SPACEOBJECTSDATA)+"' is invalid."},
    {SpaceObjectFileManager::ErrorEnum::SCHEMEFILE_INVALID,
     "The json file '"+QString(FILE_SPACEOBJECTSSCHEME)+"' is invalid."},
    {SpaceObjectFileManager::ErrorEnum::DATAFILE_NOT_OPEN,
     "The json file '"+QString(FILE_SPACEOBJECTSDATA)+"' can not be open."},
    {SpaceObjectFileManager::ErrorEnum::SCHEMEFILE_NOT_OPEN,
     "The json file '"+QString(FILE_SPACEOBJECTSSCHEME)+"' can not be open."},
    {SpaceObjectFileManager::ErrorEnum::SETSFILE_NOT_OPEN,
     "The json file '"+QString(FILE_SPACEOBJECTSSETS)+"' can not be open."},
    {SpaceObjectFileManager::ErrorEnum::INTEGRITY_ERROR,
     "The json file '"+QString(FILE_SPACEOBJECTSDATA)+"' has integrity errors."},
    {SpaceObjectFileManager::ErrorEnum::SCHEME_NOT_VALID, "The model has no valid scheme."}
};

SalaraInformation SpaceObjectFileManager::
loadSpaceObjectsDataAsObjectsList(const QString& path_data, const QString& path_scheme, SpaceObjectsList &list,
                              QString& comment, QString& version, QDateTime& versiondate)
{
    SpaceObjectModel model;
    SalaraInformation errors;
    errors = SpaceObjectFileManager::loadSpaceObjectsDataAsModel(path_data, path_scheme, model, comment,
                                                                 version, versiondate);
    if(!errors.hasError())
        list = model.getObjectsList();
    return errors;
}

SalaraInformation SpaceObjectFileManager::
loadSpaceObjectsDataAsObjects(const QString& path_data, const QString& path_scheme, SpaceObjectsMap &map,
                              QString& comment, QString& version, QDateTime& versiondate)
{
    SpaceObjectModel model;
    SalaraInformation errors;
    errors = SpaceObjectFileManager::loadSpaceObjectsDataAsModel(path_data, path_scheme, model, comment,
                                                                 version, versiondate);
    if(!errors.hasError())
        map = model.getObjectsMap();
    return errors;
}

SalaraInformation SpaceObjectFileManager::
loadSpaceObjectsDataAsModel(const QString& path_data, const QString& path_scheme, SpaceObjectModel& model, QString& comment,
                            QString& version, QDateTime& versiondate)
{

    SalaraInformation error_list = loadSpaceObjectsScheme(path_scheme, model);

    if (!error_list.hasError())
        error_list.append(loadSpaceObjectsData(path_data, model, comment, version, versiondate));

    // Return the model.
    return error_list;
}

SalaraInformation SpaceObjectFileManager::loadSpaceObjectsScheme(const QString& path_scheme, SpaceObjectModel& model)
{
    // Open the scheme.
    QFile objectmasterscheme(path_scheme);

    if(!objectmasterscheme.open(QIODevice::ReadOnly | QIODevice::Text))
        return SalaraInformation({SpaceObjectFileManager::SCHEMEFILE_NOT_OPEN ,
                                  ErrorListStringMap[ErrorEnum::SCHEMEFILE_NOT_OPEN]}, path_scheme);
    // Read all.
    QString objectmasterscheme_string = objectmasterscheme.readAll().simplified();
    objectmasterscheme.close();
    // Loads the json arrays.
    QJsonDocument objectmasterscheme_jsondocument = QJsonDocument::fromJson(objectmasterscheme_string.toUtf8());

    // Check if scheme is valid
    SalaraInformation::ErrorList error_list;

    if (objectmasterscheme_jsondocument.isNull())
    {
        error_list.append({ErrorEnum::SCHEMEFILE_INVALID, ErrorListStringMap[ErrorEnum::SCHEMEFILE_INVALID]});
    }
    else
    {
        QJsonArray objectmasterscheme_jsonarray = objectmasterscheme_jsondocument["SpaceObjectsSchemeArray"].toArray();
        model.setJsonScheme(objectmasterscheme_jsonarray);
    }

    // Return the model.
    return SalaraInformation(error_list);
}

SalaraInformation SpaceObjectFileManager::loadSpaceObjectsData(const QString& path_data, SpaceObjectModel& model,
                                                               QString& comment, QString& version,
                                                               QDateTime& versiondate)
{
    if (!model.isJsonSchemeValid())
        return SalaraInformation({SpaceObjectFileManager::SCHEME_NOT_VALID,
                                  ErrorListStringMap[ErrorEnum::SCHEME_NOT_VALID]});

    // Open the objectmaster and the scheme.
    QFile objectmasterfile(path_data);
    // Check if open.
    if(!objectmasterfile.open(QIODevice::ReadOnly | QIODevice::Text))
        return SalaraInformation({SpaceObjectFileManager::DATAFILE_NOT_OPEN,
                                  ErrorListStringMap[ErrorEnum::DATAFILE_NOT_OPEN]}, path_data);

    // Read all.
    QString objectmasterfile_string = objectmasterfile.readAll().simplified();
    objectmasterfile.close();
    // Loads the json arrays.
    QJsonDocument objectmasterfile_jsondocument = QJsonDocument::fromJson(objectmasterfile_string.toUtf8());

    // Check if scheme is valid
    SalaraInformation::ErrorList error_list;

    // Check if data file is valid
    if (objectmasterfile_jsondocument.isNull())
    {
        error_list.append({ErrorEnum::DATAFILE_INVALID, ErrorListStringMap[ErrorEnum::DATAFILE_INVALID]});
    }
    else
    {
        QJsonArray objectmasterfile_jsonarray = objectmasterfile_jsondocument["SpaceObjectsArray"].toArray();

        QJsonArray objectmasterfile_extraarray =
                objectmasterfile_jsondocument["SpaceObjectExtraparametersKeys"].toArray();
        QStringList extraparameters;
        for(const auto value : objectmasterfile_extraarray)
        {
            extraparameters.append(value.toString());
        }

        // Create the model and set the data.
        SpaceObjectModel::ErrorIndexList data_error = model.setJsonData(objectmasterfile_jsonarray);
        model.setExtraParameters(extraparameters);

        // Save version and comment.
        comment = objectmasterfile_jsondocument["Comment"].toString();
        version = objectmasterfile_jsondocument["VersionName"].toString();
        QString aux = objectmasterfile_jsondocument["VersionDate"].toString();
        versiondate = QDateTime::fromString(aux, "yyyyMMdd.hhmmss");

        // Store data integrity errors in error list
        for (const auto& error : data_error)
        {
            error_list.append(qMakePair(ErrorEnum::INTEGRITY_ERROR,
                                        ErrorListStringMap[ErrorEnum::SCHEMEFILE_INVALID]+
                              "Row: " + QString::number(error.second.row() + 1) +
                              " - Column: " +  model.headerData(error.second.column(), Qt::Horizontal).toString() +
                              " -> " + SpaceObjectModel::ErrorEnumStringMap[error.first]));
        }

    }

    // Return the model.
    return SalaraInformation(error_list);
}

SalaraInformation SpaceObjectFileManager::
loadSpaceObjectsSetsFile(const QString& path_set, QList<SpaceObjectSet*> &sets, QString& current_set)
{
    // Delete prev data.
    qDeleteAll(sets);
    sets.clear();
    current_set.clear();

    // Open the objectmaster and the scheme.
    QFile set_file(path_set);
    if(!set_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {        
        return SalaraInformation({ErrorEnum::SETSFILE_NOT_OPEN, ErrorListStringMap[ErrorEnum::SETSFILE_NOT_OPEN]});
    }

    // Read all.
    QString set_string = set_file.readAll().simplified();
    set_file.close();

    // Loads the json arrays.
    QJsonDocument setfile_document = QJsonDocument::fromJson(set_string.toUtf8());
    QJsonArray sets_array = setfile_document["SpaceObjectsSetsArray"].toArray();
    for(QJsonValueRef set : sets_array)
    {
        QStringList enabled_list, disabled_list, alwaysenabled_list, alwaysdisabled_list;
        QString name = set.toObject()["SetName"].toString();
        QJsonArray enabled = set.toObject()["ObjectsEnabled"].toArray();
        for(QJsonValueRef norad : enabled)
            enabled_list.append(norad.toString());
        SpaceObjectSet* newset = new SpaceObjectSet(name,enabled_list);
        sets.append(newset);
    }

    // Get current set
    current_set = setfile_document["CurrentSet"].toString();

    return SalaraInformation();
}


SalaraInformation SpaceObjectFileManager::
saveSpaceObjectsData(const QString& path_data, const QString& version_name, const QDateTime& version_date,
                     const QString& comment, const QJsonArray& list, const QJsonArray& extraparameters )
{
    // Creation date.
    QString creationstring = version_date.toString("yyyyMMdd.hhmmss");

    // Variables.
    QJsonObject jsonobject;
    QFile spaceobjectsdatafile(path_data);

    // Update the data.
    jsonobject.insert("VersionName", version_name);
    jsonobject.insert("VersionDate", creationstring);
    jsonobject.insert("Comment", comment);
    jsonobject.insert("SpaceObjectsArray", list);
    jsonobject.insert("SpaceObjectExtraparametersKeys", extraparameters);

    // JsonDocument (it will save in the file).
    QJsonDocument jsondocument(jsonobject);

    // Save the data.
    if(!spaceobjectsdatafile.open(QIODevice::WriteOnly))
    {
        return SalaraInformation({ErrorEnum::DATAFILE_NOT_OPEN, ErrorListStringMap[ErrorEnum::DATAFILE_NOT_OPEN]});
    }
    spaceobjectsdatafile.write(jsondocument.toJson(QJsonDocument::Indented));
    spaceobjectsdatafile.close();
    return SalaraInformation();
}


SalaraInformation SpaceObjectFileManager::
saveSpaceObjectsData(const QString&, const QString&, const QString&,
                     const QString&, const SpaceObjectsList &)
{
    //TODO
    return SalaraInformation();
}

SalaraInformation SpaceObjectFileManager::
saveSets(const QString& path_sets, const QDateTime& version_date, const QList<SpaceObjectSet*>& sets)
{
    // Variables.
    QJsonArray sets_array;
    QJsonObject set_object;
    QJsonArray enabled_array;

    // Creation date.
    QString creationstring = version_date.toString("yyyyMMdd.hhmmss");

    // Open the sets json
    QFile file(path_sets);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return SalaraInformation({ErrorEnum::SETSFILE_NOT_OPEN, ErrorListStringMap[ErrorEnum::SETSFILE_NOT_OPEN]});

    // Read all.
    QString set_string = file.readAll().simplified();
    file.close();

    // Loads the json arrays.
    QJsonDocument setfile_document = QJsonDocument::fromJson(set_string.toUtf8());

    // JsonDocument (it will save in the file).
    QJsonObject jsonobject = setfile_document.object();

    // Generate the set data.
    for(const auto& set : sets)
    { 
        set_object.insert("SetName", set->getName());
        for(const auto& norad : set->getEnabled())
            enabled_array.append(norad);
        set_object.insert("ObjectsEnabled", enabled_array);
        sets_array.append(set_object);
        enabled_array = {};
    }

    // Update the version data.
    jsonobject.insert("VersionDate", creationstring);
    jsonobject.insert("SpaceObjectsSetsArray", sets_array);

    // Update JsonDocument to save it in the file
    setfile_document.setObject(jsonobject);

    // Write the data.
    if(!file.open(QIODevice::WriteOnly))
        return SalaraInformation({ErrorEnum::SETSFILE_NOT_OPEN, ErrorListStringMap[ErrorEnum::SETSFILE_NOT_OPEN]});

    file.write(setfile_document.toJson(QJsonDocument::Indented));
    file.close();

    return SalaraInformation();
}

SalaraInformation SpaceObjectFileManager::saveCurrentSet(const QString& current_set, const QString& path_sets)
{
    QFile file(path_sets);

    // Open the sets json
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return SalaraInformation({ErrorEnum::SETSFILE_NOT_OPEN, ErrorListStringMap[ErrorEnum::SETSFILE_NOT_OPEN]});

    // Read all.
    QString set_string = file.readAll().simplified();
    file.close();

    // Loads the json arrays.
    QJsonDocument setfile_document = QJsonDocument::fromJson(set_string.toUtf8());

    // JsonDocument (it will save in the file).
    QJsonObject jsonobject = setfile_document.object();
    jsonobject.insert("CurrentSet", current_set);
    setfile_document.setObject(jsonobject);

    // Open the sets json
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return SalaraInformation({ErrorEnum::SETSFILE_NOT_OPEN, ErrorListStringMap[ErrorEnum::SETSFILE_NOT_OPEN]});

    // Write the data.
    file.write(setfile_document.toJson(QJsonDocument::Indented));
    return SalaraInformation();
}

SalaraInformation SpaceObjectFileManager::getCurrentSet(const QString& path_sets, QString& current_set)
{
    QFile file(path_sets);
    current_set.clear();

    // Open the sets json
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return SalaraInformation({ErrorEnum::SETSFILE_NOT_OPEN, ErrorListStringMap[ErrorEnum::SETSFILE_NOT_OPEN]});

    // Read all.
    QString set_string = file.readAll().simplified();
    file.close();

    // Loads the json arrays.
    QJsonDocument setfile_document = QJsonDocument::fromJson(set_string.toUtf8());

    // JsonDocument (it will save in the file).
    QJsonObject jsonobject = setfile_document.object();
    current_set = jsonobject["CurrentSet"].toString();

    return SalaraInformation();
}
