#pragma once

#include <class_salaramainwindowcontroller.h>
#include "class_spaceobjectsmanagermainwindowview.h"

#include <QDateTime>
#include <QStringListModel>

class SpaceObjectModel;
class JsonTableSortFilterProxyModel;
class SpaceObjectSet;
class SpaceObjectSearchEngine;

class SpaceObjectsManagerMainWindowController : public SalaraMainWindowController
{
    Q_OBJECT
public:
    SpaceObjectsManagerMainWindowController();
    ~SpaceObjectsManagerMainWindowController();

    void setView(SpaceObjectsManagerMainWindowView *spo_view);
    void start();

private slots:
    void slotLoadSpaceObjectsSchemeFile(const QString& path = "");
    void slotLoadSpaceObjectsDataFile(const QString& path = "");
    void slotLoadSpaceObjectsSetsFile(const QString& path = "");
    void slotAddObject();
    void slotEditObject();
    void slotRemoveObject();
    void slotSaveSpaceObjectsDataFile();
    void slotSelectionChanged();
    void slotNewSet();
    void slotLoadSet();
    void slotSaveSet();
    void slotDeleteSet();
    void slotSetCurrentSystemSet();

    void exportToCSV() const;

private:

    void makeDataBackup() const;
    void loadSystemSet();
    void saveSets();
    void loadEnabledObjectsInSet(SpaceObjectSet *set);
    void updateCounterLabels();
    void connectViewSignals();
    void connectModelSignals();
    SpaceObjectSet* getSpaceObjectSetByName(const QString& name);

    using FilterColumntype = SpaceObjectsManagerMainWindowView::FilterColumnType;
    using FilterType = SpaceObjectsManagerMainWindowView::FilterType;
    using FilterKeyType = QPair<FilterColumntype, FilterType>;

    static const QMap<FilterKeyType, QString> kMapFilterStrings;
    QMap<SpaceObjectsManagerMainWindowView::FilterColumnType, int> map_filter_column;

    SpaceObjectsManagerMainWindowView* spaceobjects_view;
    SpaceObjectModel* model;
    JsonTableSortFilterProxyModel* sortmodel;
    QStringListModel* set_listmodel;
    QList<SpaceObjectSet*> list_sets;
    QList<SpaceObjectSearchEngine*> plugins_search_engines;

    QString filedata_versionname;
    QString filedata_comment;
    QString filedata_name;
    QString system_set;
    QString loaded_set;
    QDateTime filedata_versiontime;

    // === SETTINGS =================
    // Dirs.
    QDir dir_images;
    QDir dir_spaceobjects;
    QDir dir_schemes;
    QDir dir_backup;
    bool backup_enabled;
    int max_backup_files;
};

