#pragma once

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QDateTime>
#include <QMap>
#include <QPushButton>
#include <QWidget>

#include "spcore_global.h"
#include "interface_plugin.h"
#include "interface_externaltool.h"
#include "class_salarainformation.h"
#include "class_pluginssummary.h"
#include "class_aboutpluginsdialog.h"
#include "class_spaceobject.h"

#include <utils.h>

class SP_CORE_EXPORT GlobalUtils
{
public:

    // Typedefs and static containers.
    typedef QHash<QPushButton*, bool> ButtonsState;
    static const QMap<Qt::DayOfWeek, QString> name_of_day;

    // Static class. Delete constructor.
    GlobalUtils() = delete;

    // Init app functions.
    static void initApp(const QString& app_name, const QString& app_error,
                        const QString& app_config, const QString& icon);

    // Plugins related functions.
    static SalaraInformation loadPlugins(const QDir &dir, PluginCategories cats,
                                         PluginsMultiMap& plugins, bool recursive = false);

    // Functions to disable and enable sets of buttons.
    static ButtonsState disableInterfaceButtons(QWidget* widget);
    static void restoreInterfaceButtons(QWidget* widget, const ButtonsState& state);

    // Path and directories functions.
    static void createPath(const QString &path);
    static void clearDir(const QString& path);

    // Others.
    static QString toFirstUpper(const QString &string);

    // Cospar conversion functions.
    static QString cosparToShortcospar(const QString& cospar);
    static QString shortcosparToILRSID(const QString& short_cospar);
    static QString cosparToILRSID(const QString& cospar);
    static QString shortcosparToCospar(const QString& short_cospar);

    // Date and time conversion functions.
    static QDate getSesionDateUTC();
    static void datetimeToModifiedJulianDate(const QDateTime &datetime, int& mjd, double& second);
    static QDateTime modifiedJulianDatetimeToDatetime(double mjt);
    static QDateTime julianDouble2DateTime(double julian);
    static double datetimeToModifiedJulianDatetime(const QDateTime& time);
    static double datetimeToJulianDatetime(const QDateTime& time);
    static double datetimeToReducedJulianDatetime(const QDateTime& time);
    static QDateTime timePointToQDateTime(const dpslr::common::HRTimePoint& tp, Qt::TimeSpec ts = Qt::UTC);
    static double datetimeToJ2000Datetime(const QDateTime& time);

};
