#pragma once

#include "spcore_global.h"
#include "class_spaceobjectfilemanager.h"
#include "class_salarainformation.h"

#include <QObject>
#include <QDir>

class SpaceObjectModel;
class SpaceObjectSet;

class SP_CORE_EXPORT SpaceObjectsModelLoader
{
public:

    static SpaceObjectsModelLoader& instance();

    SpaceObjectModel *getModel() const;

    const SpaceObjectsMap& objectsMap() const;
    const SpaceObjectsList& objectsList() const;
    const SpaceObjectsMap& enabledObjectsMap() const;
    const SpaceObjectsList& enabledObjectsList() const;

    QString getCurrentDataFilePath() const;
    QString getCurrentSchemeFilePath() const;
    const QString& getVersionComment() const;
    const QString &getVersionName() const;
    const QDateTime &getVersionDate() const;
    const QString &getCurrentSetName() const;

    const SalaraInformation& getLoadErrors() const;

private:

    SpaceObjectsModelLoader();
    virtual ~SpaceObjectsModelLoader();
    SpaceObjectsModelLoader( const SpaceObjectsModelLoader& ) = delete;
    SpaceObjectsModelLoader& operator=( const SpaceObjectsModelLoader& ) = delete;

    void loadSpaceObjectsDataFile();
    void loadEnabledObjectsInSet(SpaceObjectSet *set);
    void loadObjects();

    SalaraInformation load_errors;

    SpaceObjectsMap map_objects;
    SpaceObjectsMap map_objects_enabled;
    SpaceObjectsList list_objects;
    SpaceObjectsList list_objects_enabled;

    // Dirs
    QDir dir_spaceobjects;
    QDir dir_schemes;
    QString filedata_comment;
    QString version_name;
    QDateTime version_date;
    QString current_set;
    SpaceObjectModel* spaceobject_model;

};



