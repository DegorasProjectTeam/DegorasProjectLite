#include "../includes/class_spaceobjectsmodelloader.h"

#include "includes/global_texts.h"
#include "includes/class_salarasettings.h"

SpaceObjectsModelLoader::SpaceObjectsModelLoader()
{
    // Variable initialization
    this->dir_schemes.setPath( SalaraSettings::instance().getGlobalConfigString("SalaraProjectSchemeFiles/SP_SchemeFiles"));
    this->dir_spaceobjects.setPath(SalaraSettings::instance().getGlobalConfigString("SalaraProjectDataPaths/SP_SpaceObjectsData"));

    // Creates the space object model.
    this->spaceobject_model = new SpaceObjectModel(nullptr, false);

    // Load space objects into the model
    this->loadSpaceObjectsDataFile();

    // Load space objects
    this->loadObjects();
}

SpaceObjectsModelLoader::~SpaceObjectsModelLoader()
{
    delete this->spaceobject_model;
}

SpaceObjectModel *SpaceObjectsModelLoader::getModel() const
{
    return this->spaceobject_model;
}

const SpaceObjectsMap &SpaceObjectsModelLoader::objectsMap() const
{
    return this->map_objects;
}

const SpaceObjectsList &SpaceObjectsModelLoader::objectsList() const
{
    return this->list_objects;
}

const SpaceObjectsMap &SpaceObjectsModelLoader::enabledObjectsMap() const
{
    return this->map_objects_enabled;
}

const SpaceObjectsList &SpaceObjectsModelLoader::enabledObjectsList() const
{
    return this->list_objects_enabled;
}

void SpaceObjectsModelLoader::loadSpaceObjectsDataFile()
{
    QList<SpaceObjectSet*> list_sets;

    QString pathfile = this->dir_spaceobjects.path()+ '/' + FILE_SPACEOBJECTSDATA;
    QString pathscheme = this->dir_schemes.path() + '/' + FILE_SPACEOBJECTSSCHEME;
    QString pathsets = this->dir_spaceobjects.path() + '/' + FILE_SPACEOBJECTSSETS;

    // Loads the model, and the sets, then check for errors and set it in the table view.
    SalaraInformation errors =
            SpaceObjectFileManager::loadSpaceObjectsDataAsModel(pathfile, pathscheme,
                                                           *this->spaceobject_model, this->filedata_comment,
                                                           this->version_name, this->version_date);

    errors.append(SpaceObjectFileManager::loadSpaceObjectsSetsFile(pathsets, list_sets, this->current_set));


    if (!errors.hasError())
    {
        // If there is not an error in model and sets load, then initialize elements in GUI that depends on it
        // First set disabled all objects that are not always enabled or disabled
        this->spaceobject_model->disableAll();
        // Look for current set, if it exists
        if (!this->current_set.isEmpty())
        {
            auto it = std::find_if(list_sets.begin(), list_sets.end(),
                         [set_name = this->current_set](const auto& set){return set->getName() == set_name;});

            // Apply the current set
            if (it != list_sets.end())
                loadEnabledObjectsInSet(*it);
        }

    }

    this->load_errors = errors;

}

QString SpaceObjectsModelLoader::getCurrentDataFilePath() const
{
    return this->dir_spaceobjects.path() + FILE_SPACEOBJECTSDATA;
}

QString SpaceObjectsModelLoader::getCurrentSchemeFilePath() const
{
    return this->dir_schemes.path() + FILE_SPACEOBJECTSSCHEME;
}

const QString& SpaceObjectsModelLoader::getVersionComment() const
{
    return this->filedata_comment;
}

const QString& SpaceObjectsModelLoader::getVersionName() const
{
    return this->version_name;
}

const QDateTime& SpaceObjectsModelLoader::getVersionDate() const
{
    return this->version_date;
}

const QString& SpaceObjectsModelLoader::getCurrentSetName() const
{
    return this->current_set;
}

const SalaraInformation& SpaceObjectsModelLoader::getLoadErrors() const
{
    return this->load_errors;
}

void SpaceObjectsModelLoader::loadEnabledObjectsInSet(SpaceObjectSet* set)
{
    int enablement_column = this->spaceobject_model->findColumnSectionByIndex("EnablementPolicy");
    int norad_column = this->spaceobject_model->findColumnSectionByIndex("NORAD");
    // Set enabled objects in set as ENABLED.
    QStringList enabled_objects = set->getEnabled();
    if(enabled_objects.isEmpty())
        return;
    // Filter model by enabled norads. Prepend \\b0* and append \\b to save leading zeros and ensure it is a word.
    JsonTableSortFilterProxyModel sortmodel;
    sortmodel.setSourceModel(this->spaceobject_model);
    sortmodel.setFilter(norad_column, "\\b0*" + enabled_objects.join("\\b|\\b0*") + "\\b");
    for (int i = 0; i < sortmodel.rowCount(); i++)
    {
        QModelIndex index = sortmodel.index(i, enablement_column);
        sortmodel.setData(index, SpaceObject::EnablementPolicy::ENABLED, Qt::UserRole);
    }

}

void SpaceObjectsModelLoader::loadObjects()
{
    if (!this->spaceobject_model)
        return;

    this->map_objects = this->spaceobject_model->getObjectsMap();
    this->list_objects = this->map_objects.values();

    for(const auto& spaceobject: std::as_const(this->list_objects))
    {
        if(spaceobject->getEnablementPolicy()==SpaceObject::EnablementPolicy::ENABLED ||
                spaceobject->getEnablementPolicy()==SpaceObject::EnablementPolicy::ALWAYS_ENABLED)
        {
            this->map_objects_enabled[spaceobject->getNorad()] = spaceobject;
            this->list_objects_enabled.push_back(spaceobject);
        }
    }
}

SpaceObjectsModelLoader &SpaceObjectsModelLoader::instance()
{
    static SpaceObjectsModelLoader inst;
    return inst;
}
