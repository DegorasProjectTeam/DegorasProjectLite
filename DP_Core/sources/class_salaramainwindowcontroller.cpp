#include "includes/class_salaramainwindowcontroller.h"
#include "includes/class_salarasettings.h"
#include "includes/class_pluginssummary.h"
#include "includes/class_aboutpluginsdialog.h"
#include "includes/class_aboutdialog.h"
#include "includes/class_globalutils.h"
#include "includes/class_guiloader.h"

#include <class_salaramainwindowview.h>

#include <QDesktopServices>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QApplication>
#include <QUrl>


SalaraMainWindowController::SalaraMainWindowController(QObject *parent) :
    QObject(parent),
    dir_plugins(SalaraSettings::instance().getGlobalConfigString("SalaraProjectPluginPaths/SP_Plugins")),
    view(nullptr),
    app_license_filepath(":/License"),
    app_about_filepath(":/About"),
    app_icon(QApplication::windowIcon())
{

}

SalaraMainWindowController::~SalaraMainWindowController()
{
    for (const auto& plugin : std::as_const(this->plugins))
    {
        delete plugin;
    }
}

void SalaraMainWindowController::setView(SalaraMainWindowView* view)
{
    // If there was a previous view, disconnect signals
    if (this->view)
    {

        for(const auto& connection : std::as_const(this->connections))
            QObject::disconnect(connection);

        for (const auto& connection : std::as_const(this->external_tools_connections))
            QObject::disconnect(connection);

        this->connections.clear();
        this->external_tools_connections.clear();
    }

    this->view = view;
    this->view->setInstitution(SalaraSettings::instance().getGlobalConfigString("Institution/SP_Institution"));

    this->connectViewSignals();
}

SalaraInformation SalaraMainWindowController::loadPlugins(PluginCategories load_types)
{
    // Load plugins of selected categories and return result
    return GlobalUtils::loadPlugins(this->dir_plugins, load_types, this->plugins, true);
}

void SalaraMainWindowController::loadExternalToolsActionsToView()
{
    if (!view)
        return;

    QMenu* external_tool_menu = this->view->getMenu(SalaraMainWindowView::MenuType::EXTERNAL_TOOLS_MENU);
    if (external_tool_menu)
    {
        // If there were previous external tool actions or connections, delete or disconnect them
        for (const auto& connection : std::as_const(this->external_tools_connections))
            QObject::disconnect(connection);

        this->external_tools_connections.clear();


        QList<QAction*> actions = external_tool_menu->actions();
        for (const auto& action : actions)
        {
            external_tool_menu->removeAction(action);
            delete action;
        }

        // Create actions for current external tools
        QList<QAction*> external_tool_actions;
        for (auto&& plugin : this->plugins.values(PluginCategory::EXTERNAL_TOOL))
        {
            QAction* action = new QAction(plugin->getPluginName());
            action->setData(QVariant::fromValue(qobject_cast<ExternalTool*>(plugin)));
            external_tools_connections.append(QObject::connect(action, &QAction::triggered, this->view,
                             [tool_plugin = qobject_cast<ExternalTool*>(plugin), view = this->view]
            {
                view->setEnabled(false);
                tool_plugin->execExternalTool();
                view->setEnabled(true);
            }));
            external_tool_actions.push_back(action);
        }
        external_tool_menu->addActions(external_tool_actions);
    }
}


void SalaraMainWindowController::connectViewSignals()
{
    if (!this->view)
        return;

    QAction* action = this->view->getAction(SalaraMainWindowView::ActionType::PLUGINS_SUMMARY);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, [this]
        {
            GuiLoader::exec([this]{
                this->view->setEnabled(false);
                PluginsSummary pluginssummary(this->plugins.values(), this->view);
                pluginssummary.setEnabled(true);
                pluginssummary.exec();
                this->view->setEnabled(true);
            });
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::ABOUT_PLUGINS);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, [this]
        {
            GuiLoader::exec([this]{
                AboutPluginsDialog pluginsabout(this->plugins, this->view);
                this->view->setEnabled(false);
                pluginsabout.setEnabled(true);
                pluginsabout.exec();
                this->view->setEnabled(true);
                QMenu* external_tools_menu = this->view->getMenu(SalaraMainWindowView::MenuType::EXTERNAL_TOOLS_MENU);
                if (external_tools_menu)
                    for (auto&& action : external_tools_menu->actions())
                    {
                        ExternalTool* tool = action->data().value<ExternalTool*>();
                        if (tool)
                            action->setEnabled(tool->isEnabled());
                    }
            });
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::OPEN_PLUGINS_FOLDER);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, [this]
        {
            QDesktopServices::openUrl(QUrl(this->dir_plugins.path(), QUrl::TolerantMode));
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::ABOUT_PROJECT);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, [this]{
            GuiLoader::exec([this]{
                this->view->setEnabled(false);
                AboutDialog::aboutSalaraProject(this->view);
                this->view->setEnabled(true);
            });
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::ABOUT_APP);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, [this]{
            GuiLoader::exec([this]{
                this->view->setEnabled(false);
                AboutDialog about_spaceobjectsmanager(this->app_license_filepath,
                                                      this->app_about_filepath, this->app_icon, this->view);
                about_spaceobjectsmanager.setEnabled(true);
                about_spaceobjectsmanager.exec();
                this->view->setEnabled(true);
            });
        }));

    // Set connections for Configurations menu
    action = this->view->getAction(SalaraMainWindowView::ActionType::CONFIGURE_APP);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, []
        {
            QDesktopServices::openUrl(QUrl(SalaraSettings::instance().getApplicationConfigFilePath(), QUrl::TolerantMode));
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::CONFIGURE_PROJECT);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, []
        {
            QDesktopServices::openUrl(QUrl(SalaraSettings::instance().getGlobalConfigFilePath(), QUrl::TolerantMode));
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::OPEN_PROJECT_ROOT_FOLDER);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, []
        {
            QString root_path = SalaraSettings::instance().getGlobalConfigString("SalaraProjectRootPath/SP_Root");
            QDesktopServices::openUrl(QUrl(root_path, QUrl::TolerantMode));
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::OPEN_PROJECT_CONFIG_FOLDER);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this, []
        {
            QString config_path = SalaraSettings::instance().getGlobalConfigString("SalaraProjectConfigPaths/SP_ConfigFiles");
            QDesktopServices::openUrl(QUrl(config_path, QUrl::TolerantMode));
        }));

    action = this->view->getAction(SalaraMainWindowView::ActionType::OPEN_PROJECT_DATA_FOLDER);
    if (action)
        connections.append(QObject::connect(action, &QAction::triggered, this,[]
        {
            QString data_path = SalaraSettings::instance().getGlobalConfigString("SalaraProjectDataPaths/SP_DataFiles");
            QDesktopServices::openUrl(QUrl(data_path, QUrl::TolerantMode));
        }));
}


