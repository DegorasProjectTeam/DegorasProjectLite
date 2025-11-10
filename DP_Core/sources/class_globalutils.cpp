#include "includes/global_texts.h"
#include "includes/class_salarasettings.h"
#include "includes/class_globalutils.h"
#include "includes/class_salarainformation.h"
#include "includes/interface_cpfdownloadengine.h"
#include "includes/interface_externaltool.h"

#include <QCoreApplication>
#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QLockFile>
#include <QStyleFactory>
#include <QIcon>
#include <QtMath>
#include <QPluginLoader>
#include <QMenu>

#include <chrono>

#include <utils.h>

QMap<Qt::DayOfWeek, QString> const GlobalUtils::name_of_day{{Qt::Monday, "Monday"},
                                                 {Qt::Tuesday, "Tuesday"},
                                                 {Qt::Wednesday, "Wednesday"},
                                                 {Qt::Thursday, "Thursday"},
                                                 {Qt::Friday, "Friday"},
                                                 {Qt::Saturday, "Saturday"},
                                                 {Qt::Sunday, "Sunday"}};


void GlobalUtils::initApp(const QString& app_name, const QString& app_error,
             const QString& app_config, const QString& icon)
{
    // Variables.
    QString globalconfiglink_filename = FILE_GLOBALCONFIGLINK;
    QString globalconfig_filename = FILE_GLOBALCONFIG;
    QString stationdata_filename = FILE_STATIONDATA;
    QString globalconfiglink_filepath, globalconfig_filepath, appconfig_filepath, stationdata_filepath;
    QDir globalconfiglink_dir, globalconfig_dir, appconfig_dir, stationdata_dir;

    // Init names and icons.
    QApplication::setWindowIcon(QIcon(icon));
    QCoreApplication::setOrganizationName(NAME_ORGANIZATION);
    QCoreApplication::setOrganizationDomain(NAME_ORGANIZATION);
    QCoreApplication::setApplicationName(app_name);

    // Load styles, palette, fonts,...
    QPalette palette;
    QFile file(FILE_STYLESHEET_DEFAULT);
    file.open(QFile::ReadOnly);
    QString stylesheet = QLatin1String(file.readAll());
    file.close();
    QStyle *style = QStyleFactory::create("fusion");
    QApplication::setStyle(style);
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::ButtonText, Qt::white);
    QApplication::setPalette(palette);
    QFontDatabase::addApplicationFont(":/fonts/Open_Sans/OpenSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Open_Sans/OpenSans-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Open_Sans/OpenSans-ExtraBold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Open_Sans/OpenSans-SemiBold.ttf");
    QApplication::setFont(QFont("Open Sans", 8, QFont::Normal));
    qApp->setStyleSheet(stylesheet);

    // Locks to check if other instance of the app is running.
    QLockFile lockFile(QDir::temp().absoluteFilePath(app_name+".lock"));
    if(!lockFile.tryLock(100))
    {
        SalaraInformation::showError(app_error, app_name +
                                     " is already running.\nAllowed to run only one instance of the application.", "",
                                     SalaraInformation::CRITICAL);
        exit(-1);
    }

    // Check and loads the SALARA Project configuration file.
    // Get dir and name of the global config file from the global config path file
    globalconfiglink_dir.setPath(QApplication::instance()->applicationDirPath());
    globalconfiglink_dir.cdUp();
    globalconfiglink_filepath = globalconfiglink_dir.path()+'/'+globalconfiglink_filename;
    if(!QFile(globalconfiglink_filepath).exists())
    {
        SalaraInformation::showError(app_error,
                                     "Global configuration file link '"+globalconfiglink_filename+"' not found in the "
                                     "deployment directory (check it clicking 'Show Details...').",
                                     globalconfiglink_dir.path(), SalaraInformation::CRITICAL);
        exit(-1);
    }

    // Get the global config file path
    QSettings salara_globalconfig_path(globalconfiglink_dir.path()+"/"+globalconfiglink_filename, QSettings::IniFormat);
    globalconfig_dir.setPath(salara_globalconfig_path.value("SalaraGlobalConfig/GlobalConfigLink").toString());
    globalconfig_filepath = globalconfig_dir.path()+"/"+globalconfig_filename;
    if(!QFile(globalconfig_filepath).exists())
    {
        SalaraInformation::showError(app_error,
                                     "Global configuration file '"+globalconfig_filename+"' not found in the "
                                     "main directory (check it clicking 'Show Details...').", globalconfig_dir.path(),
                                     SalaraInformation::CRITICAL);
        exit(-1);
    }
    SalaraSettings::instance().setGlobalConfig(globalconfig_filepath);

    // Get APP config file.
    appconfig_dir.setPath(SalaraSettings::instance().getGlobalConfigString("SalaraProjectConfigPaths/SP_ConfigFiles"));
    appconfig_filepath = appconfig_dir.path()+"/"+app_config;

    if(!QFile(appconfig_filepath).exists())
    {
        SalaraInformation::showError(app_error, app_name +
                                     " App configuration file '"+QString(app_config)+"' not found in the config "
                                     "directory (check it clicking 'Show Details...').", appconfig_dir.path(),
                                     SalaraInformation::CRITICAL);
        exit(-1);
    }
    SalaraSettings::instance().setApplicationConfig(appconfig_filepath);



    // Create all the paths.
    SalaraSettings::instance().getGlobalConfig()->beginGroup("SalaraProjectDataPaths");
    for(auto&& key : SalaraSettings::instance().getGlobalConfig()->allKeys())
        GlobalUtils::createPath(SalaraSettings::instance().getGlobalConfig()->value(key).toString());
    SalaraSettings::instance().getGlobalConfig()->endGroup();
    SalaraSettings::instance().getGlobalConfig()->beginGroup("SalaraProjectConfigPaths");
    for(auto&& key : SalaraSettings::instance().getGlobalConfig()->allKeys())
        GlobalUtils::createPath(SalaraSettings::instance().getGlobalConfig()->value(key).toString());
    SalaraSettings::instance().getGlobalConfig()->endGroup();
    SalaraSettings::instance().getGlobalConfig()->beginGroup("SalaraProjectPluginPaths");
    for(auto&& key : SalaraSettings::instance().getGlobalConfig()->allKeys())
        GlobalUtils::createPath(SalaraSettings::instance().getGlobalConfig()->value(key).toString());
    SalaraSettings::instance().getGlobalConfig()->endGroup();
    SalaraSettings::instance().getGlobalConfig()->beginGroup("SalaraProjectBackupPaths");
    for(auto&& key : SalaraSettings::instance().getGlobalConfig()->allKeys())
        GlobalUtils::createPath(SalaraSettings::instance().getGlobalConfig()->value(key).toString());
    SalaraSettings::instance().getGlobalConfig()->endGroup();

    // Load the station data.
    stationdata_dir.setPath(SalaraSettings::instance().getGlobalConfigString("SalaraProjectDataPaths/SP_StationData"));
    if(!stationdata_dir.exists(stationdata_filename))
    {
        SalaraInformation::showError(app_error, "Station data file '"+stationdata_filename+"' not found in "
                                     "the data directory (check it clicking 'Show Details...').", stationdata_dir.path(),
                                     SalaraInformation::CRITICAL);
        exit(-1);
    }

    SalaraInformation error = SalaraSettings::instance().setStationData(stationdata_dir.path()+"/"+stationdata_filename);
    if(error.hasError())
    {
        error.showErrors(app_error, SalaraInformation::CRITICAL);
        exit(-1);
    }
}


SalaraInformation GlobalUtils::loadPlugins(const QDir& dir, PluginCategories cats,
                                           PluginsMultiMap& plugins, bool recursive)
{
    // Operation result.
    SalaraInformation result;

    // Recursive call for load all the plugins in each subdir.
    if(recursive)
        for(auto&& internal_dir : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
            result.append(loadPlugins(QDir(dir.path()+'/'+internal_dir), cats, plugins, true));

    // Load each plugin in the folder (if it has the correct category).
    for(auto&& file : dir.entryList(QDir::Files))
    {
        // Variables and loader.
        QPluginLoader plugin_loader(dir.path()+'/'+file);
        SPPlugin* sp_plugin;

        // If load fails, return the error.
        if(!plugin_loader.load())
        {
            // Reformat the error code.
            QString line = plugin_loader.errorString();
            QStringList splitter = line.split("library");

            if (splitter.size() > 1)
            {
                line = splitter[1];
                int index = line.lastIndexOf(':');
                line = line.remove(index, 1);
                line = line.insert(index, " ->");
            }
            result.append(SalaraInformation({0, line}));
            plugin_loader.unload();
            continue;
        }

        // If loads is ok, get the instance.
        sp_plugin = qobject_cast<SPPlugin*>(plugin_loader.instance());

        // If the cast fails, return the error.
        if(sp_plugin == nullptr)
        {
            result.append(SalaraInformation({0, file + " -> " + TEXT_ERROR_PLUGIN_GENERIC_CAST_FAIL}));
            plugin_loader.unload();
            continue;
        }

        // If the cast is ok, set the meta data and check if plugin has selected categories.
        sp_plugin->setMetaData(plugin_loader.metaData());
        sp_plugin->setFilePath(file);

        if(!cats.testFlag(sp_plugin->getPluginCategory()))
        {
            plugin_loader.unload();
            continue;
        }

        // Check if the plugin is already loaded.
        auto it = std::find_if(plugins.begin(), plugins.end(), [sp_plugin](const auto& plugin)
        {
            return (sp_plugin->getPluginName() == plugin->getPluginName() &&
                    sp_plugin->getPluginVersion() == plugin->getPluginVersion());
        });
        if(it != plugins.end())
        {
            result.append(SalaraInformation({0, file + " -> " + TEXT_ERROR_PLUGIN_ALREADY_LOADED}));
            plugin_loader.unload();
            continue;
        }

        // Check if the specific plugin cast is ok.
        if(!checkPluginSpecificCast(sp_plugin))
        {
            result.append(SalaraInformation({0, file + " -> " + TEXT_ERROR_PLUGIN_SPECIFIC_CAST_FAIL}));
            plugin_loader.unload();
            continue;
        }

        // If all is ok, set the plugin enabled and add to a action if neccesary.
        sp_plugin->setEnabled(true);
        plugins.insert(sp_plugin->getPluginCategory(), sp_plugin);
    }

    // All ok, return empty errors.
    return result;
}

void GlobalUtils::createPath(const QString& path)
{
    if(!path.isEmpty() && !QDir(path).exists())
        QDir().mkdir(path);
}


void GlobalUtils::clearDir(const QString& path)
{
    QDir dir(path);
    dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
    foreach( QString dirItem, dir.entryList() )
        dir.remove( dirItem );
    dir.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );
    foreach( QString dirItem, dir.entryList() )
    {
        QDir subDir( dir.absoluteFilePath( dirItem ) );
        subDir.removeRecursively();
    }
}


QString GlobalUtils::toFirstUpper(const QString& string)
{
    QStringList splitter = string.simplified().trimmed().split(" ");
    QString final;
    for(auto&& str : splitter)
    {
        str = str.toLower();
        str[0] = str[0].toUpper();
    }
    final = splitter.join(" ");
    return final;
}


// 1929-987A -> 29987A
QString GlobalUtils::cosparToShortcospar(const QString& cospar)
{
    QString aux(cospar);
    QString short_cospar = aux.remove(0, 2);
    short_cospar.remove("-");
    return short_cospar;
}

// 29987A -> 2998701
QString GlobalUtils::shortcosparToILRSID(const QString& short_cospar)
{
    QString abc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString aux, roa_id;
    roa_id = "";
    int number;
    foreach(QChar a, short_cospar)
        if(a.isLetter())
            aux=aux+a;
        else
            roa_id = roa_id+a;
    if(aux.size()==1)
    {
        number = abc.indexOf(aux)+1;
        if(number<10)
            roa_id = roa_id + "0";
        roa_id = roa_id + QString::number(number);
    }
    else if(aux.size()==2)
    {
        number = (abc.indexOf(aux[0])+1)*26;
        number = number + abc.indexOf(aux[1])+1;
        roa_id = roa_id + QString::number(number);
    }
    return roa_id;
}


// 1929-987A -> 2998701
QString GlobalUtils::cosparToILRSID(const QString& cospar)
{
    QString scospar = cosparToShortcospar(cospar);
    return shortcosparToILRSID(scospar);
}


QString GlobalUtils::shortcosparToCospar(const QString& short_cospar)
{
    QString cospar;
    if(short_cospar.leftRef(2).toInt()>=57)
        cospar.append("19");
    else
        cospar.append("20");
    cospar.append(short_cospar.leftRef(2));
    cospar.append("-");
    cospar.append(short_cospar.midRef(2,3));
    cospar.append(short_cospar.midRef(5));
    return cospar;
}

QDate GlobalUtils::getSesionDateUTC()
{
    // Determinación del día de la sesión. Si es mayor a 7 horas (han pasado 7 horas desde el inicio del día)
    // entonces la fecha a tratar es el propio dia. Si es menor a 6 horas, estamos en la sesión que comenzó la tarde
    // del dia anterior. No creo que nunca se llegue a dar el caso en el que este procedimiento falle.
    // TODO: test pending

    QDateTime sesion_dt(QDateTime::currentDateTimeUtc().date(), QTime(0,0));
    if (sesion_dt.secsTo(QDateTime::currentDateTimeUtc()) < 25200)
        sesion_dt = sesion_dt.addDays(-1);
    return sesion_dt.date();
}


// Datetime conversors.
void GlobalUtils::datetimeToModifiedJulianDate(const QDateTime &datetime, int& mjd, double& second)
{
    double unix_seconds = datetime.toSecsSinceEpoch();
    mjd = static_cast<int>((unix_seconds/86400.0) + 2440587.5 - 2400000.5);
    second = datetime.time().msecsSinceStartOfDay()/1000.0;
}

double GlobalUtils::datetimeToModifiedJulianDatetime(const QDateTime &datetime)
{
    double unix_seconds = datetime.toSecsSinceEpoch();
    double mjd = (unix_seconds/86400.0) + 2440587.5 - 2400000.5;
    return mjd;
}

double GlobalUtils::datetimeToJulianDatetime(const QDateTime &datetime)
{
    double unix_seconds = datetime.toSecsSinceEpoch();
    double jd = (unix_seconds/86400.0) + 2440587.5;
    return jd;
}

double GlobalUtils::datetimeToReducedJulianDatetime(const QDateTime &datetime)
{
    double unix_seconds = datetime.toSecsSinceEpoch();
    double rjd = (unix_seconds/86400.0) + 2440587.5 - 2400000;
    return rjd;
}


QDateTime GlobalUtils::timePointToQDateTime(const dpslr::common::HRTimePoint &tp, Qt::TimeSpec ts)
{
    using namespace std::chrono;
    return QDateTime::fromSecsSinceEpoch(duration_cast<seconds>(tp.time_since_epoch()).count(), ts);
}


double GlobalUtils::datetimeToJ2000Datetime(const QDateTime &datetime)
{
    double jd = datetimeToJulianDatetime(datetime);
    return (jd + dpslr::utils::kJulianToJ2000);
}


QDateTime GlobalUtils::modifiedJulianDatetimeToDatetime(double mjt)
{
    // MJD to JD.
    mjt+=2400000.5;
    // The day number is the integer part of the date.
    int julian_day = qFloor(mjt);
    QDate date = QDate::fromJulianDay(julian_day);
    // The fraction is the time of day.
    double julian_msecs = (mjt - static_cast<double>(julian_day)) * 86400.0 * 1000.0;
    // Add the seconds.
    QTime time = QTime(12, 0, 0, 0).addMSecs(qRound(julian_msecs));
    if(time.hour()>=0 && time.hour()<12)
          date = date.addDays(1);
    return QDateTime(date, time, Qt::UTC);
}


QDateTime GlobalUtils::julianDouble2DateTime(double julian)
{
    // The day number is the integer part of the date
    int julianDays = qFloor(julian);
    QDate d = QDate::fromJulianDay(julianDays);
    // The fraction is the time of day
    double julianMSecs = (julian - static_cast<double>(julianDays)) * 86400.0 * 1000;
    // Julian days start at noon (12:00 UTC)
    QTime t = QTime(12, 0, 0, 0).addMSecs(qRound(julianMSecs));
    // Corrección (el dia juliano empieza a las 12)
    if(t.hour()>=0 && t.hour()<12)
        d = d.addDays(1);
    return QDateTime(d, t, Qt::UTC);
}


GlobalUtils::ButtonsState GlobalUtils::disableInterfaceButtons(QWidget *widget)
{
   GlobalUtils::ButtonsState buttons_state;
   QList<QPushButton *> list_sources_buttons = widget->findChildren<QPushButton *>(QString(), Qt::FindDirectChildrenOnly);
   for(QPushButton* button : list_sources_buttons)
   {
       buttons_state.insert(button, button->isEnabled());
       button->setEnabled(false);
   }
   return  buttons_state;
}


void GlobalUtils::restoreInterfaceButtons(QWidget *widget, const GlobalUtils::ButtonsState& state)
{
   QList<QPushButton *> list_sources_buttons = widget->findChildren<QPushButton *>(QString(), Qt::FindDirectChildrenOnly);
   for(QPushButton* button : list_sources_buttons)
       button->setEnabled(state[button]);
}















