#include "includes/interface_plugin.h"
#include "includes/interface_spaceobjectsearchengine.h"
#include "includes/interface_cpfdownloadengine.h"
#include "includes/interface_externaltool.h"
#include "includes/interface_tledownloadengine.h"
#include "includes/interface_tle_propagator.h"

#include <dpslr_math.h>


SPPlugin::SPPlugin(PluginCategory category) : category_(category) {}

void SPPlugin::setMetaData(const QJsonObject &metadata)
{
    this->name =  metadata["MetaData"].toObject()["Name"].toString();
    this->shortname =  metadata["MetaData"].toObject()["ShortName"].toString();
    this->version = metadata["MetaData"].toObject()["Version"].toString();
    this->copyright = metadata["MetaData"].toObject()["Copyright"].toString();
    this->metadata = metadata;
}

void SPPlugin::setFilePath(const QString &path){this->filedir.setPath(path);}

void SPPlugin::setEnabled(bool enabled){this->enabled=enabled;}

const QString &SPPlugin::getPluginName() const {return this->name;}

const QString &SPPlugin::getPluginShortName() const {return this->shortname;}

const QString &SPPlugin::getPluginVersion() const {return this->version;}

const QString &SPPlugin::getPluginCopyright() const {return this->copyright;}

PluginCategory SPPlugin::getPluginCategory() const {return this->category_;}


const QDir &SPPlugin::getPluginDir() const {return this->filedir;}

const bool &SPPlugin::isEnabled() const {return this->enabled;}
class ResultsSender;
bool checkPluginSpecificCast(SPPlugin* plugin)
{
    bool result = false;
    switch (plugin->getPluginCategory())
    {
    case PluginCategory::SPACE_OBJECT_SEARCH_ENGINE:
        result = qobject_cast<SpaceObjectSearchEngine*>(plugin);
        break;

    case PluginCategory::CPF_DOWNLOAD_ENGINE:
        result = qobject_cast<CPFDownloadEngine*>(plugin);
        break;
    case PluginCategory::TLE_DOWNLOAD_ENGINE:
        result = qobject_cast<TLEDownloadEngine*>(plugin);
        break;
    case PluginCategory::EXTERNAL_TOOL:
        result = qobject_cast<ExternalTool*>(plugin);
        break;

    case PluginCategory::TLE_PROPAGATOR:
        result = qobject_cast<TLEPropagator*>(plugin);
        break;

    }
    return result;
}

const QMap<PluginCategory, QString> PluginCategoryEnumMap =
{
    {PluginCategory::SPACE_OBJECT_SEARCH_ENGINE, "Space Object Search Engine"},
    {PluginCategory::LASER_SYSTEM_CONTROLLER, "Laser System Controller"},
    {PluginCategory::CPF_DOWNLOAD_ENGINE, "CPF Download Engine"},
    {PluginCategory::TLE_DOWNLOAD_ENGINE, "TLE Download Engine"},
    {PluginCategory::EXTERNAL_TOOL, "External Tool"},
    {PluginCategory::RGG_CONTROLLER, "Station Controller Client"},
    {PluginCategory::RESULTS_FORMAT, "Results Format"},
    {PluginCategory::RANGE_SOURCE, "Ranges Source"},
    {PluginCategory::EVENT_TIMER, "Event Timer"},

    {PluginCategory::METEO_DATA_SOURCE, "Meteo Data Source"},
    {PluginCategory::RESULTS_SENDER, "Results Sender"},
    {PluginCategory::MOUNT_SYSTEM_CONTROLLER, "Mount System Controller"},
    {PluginCategory::TLE_PROPAGATOR, "TLE Propagator"},
    {PluginCategory::DOME_SYSTEM_CONTROLLER, "Dome System Controller"}
};



QString pluginCategoryString(SPPlugin *plugin)
{
    return pluginCategoryString(plugin->getPluginCategory());
}

QString pluginCategoryString(PluginCategory category)
{
    auto it = PluginCategoryEnumMap.constFind(category);
    return it != PluginCategoryEnumMap.cend() ? it.value() : "";
}
