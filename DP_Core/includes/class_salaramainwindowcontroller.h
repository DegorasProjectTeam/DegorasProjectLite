#pragma once

#include "class_salarainformation.h"
#include "interface_plugin.h"
#include "spcore_global.h"

#include <QObject>
#include <QIcon>

class SalaraMainWindowView;

class SP_CORE_EXPORT SalaraMainWindowController : public QObject
{
    Q_OBJECT
public:
    explicit SalaraMainWindowController(QObject *parent = nullptr);
    virtual ~SalaraMainWindowController();

protected:
    void setView(SalaraMainWindowView *view);
    SalaraInformation loadPlugins(PluginCategories load_types);
    void loadExternalToolsActionsToView();

    PluginsMultiMap plugins;
    QDir dir_plugins;
    SalaraMainWindowView* view;
    QString app_license_filepath;
    QString app_about_filepath;
    QIcon app_icon;

private:
    void connectViewSignals();
    QList<QMetaObject::Connection> connections;
    QList<QMetaObject::Connection> external_tools_connections;
};

