#pragma once

#include <class_salaramainwindowview.h>
#include <class_spaceobject.h>

#include <QModelIndexList>
#include <QItemSelectionModel>
#include <QMessageBox>

namespace Ui {
class SpaceObjectsManagerMainWindowView;
}

class FormSave;
class FormSatellite;
class SpaceObjectSearchEngine;
class SpaceObjectModel;


class SpaceObjectsManagerMainWindowView : public SalaraMainWindowView
{
    Q_OBJECT

public:


    enum FilterColumnType
    {
        DEBRIS_FILTER,
        LRR_FILTER,
        ILRS_FILTER,
        EN_POLICY_FILTER
    };
    enum FilterType
    {
        ALL_FILTER,
        YES_FILTER,
        NO_FILTER
    };

    explicit SpaceObjectsManagerMainWindowView(QWidget *parent = nullptr);
    ~SpaceObjectsManagerMainWindowView() override;


    QItemSelectionModel* getTableSelectionModel();
    int getCurrentSelectedSetIndex() const;
    QSize getSpaceObjectPictureSize() const;

public slots:
    void setModel(QAbstractItemModel* table_model, QAbstractItemModel *sets_model);
    void clearGUI();
    void setCurrentSetIndex(int index);
    void setLoadedNumber(int num);
    void setVisibleNumber(int num);
    void setDBVersionName(const QString& version);
    void setDBVersionDate(const QString& date);
    void setEnabledNumber(int num);
    void setSaveObjectsEnabled(bool enabled);
    void setSaveSetsEnabled(bool enabled);
    void setCurrentSystemSet(const QString& set_name);
    void setCurrentLoadedSet(const QString& set_name);
    void setNewSetEnabled(bool enabled);
    void setLoadDeleteSetSystemSetEnabled(bool enabled);
    void setLoadDeleteSetEnabled(bool enabled);
    void setCopyActionsEnabled(bool enabled);
    void setEnPolicyOperationsEnabled(bool enabled);
    void setEditSpaceObjectEnabled(bool enabled);
    void setRemoveSpaceObjectEnabled(bool enabled);
    void setSpaceObjectPicture(const QPixmap& pixmap);
    int execFormSatellite(const QList<SpaceObjectSearchEngine*>& plugins, SpaceObjectModel* model, int index = 0);

signals:
    void signalEditObject();
    void signalAddObject();
    void signalRemoveObjects();
    void signalSaveSpaceObjects();
    void signalLoadSpaceObjects();
    void signalNewSet();
    void signalLoadSet();
    void signalSaveSet();
    void signalDeleteSet();
    void signalSetSystemSet();
    void signalSelectionChanged();
    void signalFilterStringChanged(const QString& filter);
    void signalEditEnablementPolicy(SpaceObject::EnablementPolicy policy);
    void signalCopyTriggered(const QString& column);
    void signalOpenSchemeFile();
    void signalOpenDataFile();
    void signalSetFilter(FilterColumnType filter_column, FilterType filter_type);

    void signalExportToCSV();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:

    Ui::SpaceObjectsManagerMainWindowView *m_ui;
    FormSatellite* form_satellite;
    FormSave* form_save;

    QMenu* menu_data_;
    QMenu* menu_export_;
    QAction *export_csv_;

    void makeConnections();
};

