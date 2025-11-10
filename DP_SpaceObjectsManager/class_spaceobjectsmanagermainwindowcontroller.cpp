#include "class_spaceobjectsmanagermainwindowcontroller.h"
#include "interface_spaceobjectsearchengine.h"
#include "form_satellite.h"
#include "form_save.h"

#include <class_spaceobjectfilemanager.h>
#include <class_salarasettings.h>
#include <global_texts.h>
#include <class_guiloader.h>

#include <QInputDialog>
#include <QDesktopServices>
#include <QFileDialog>
#include <QClipboard>
#include <QImageReader>
#include <QMetaObject>
#include <QThread>
#include <QApplication>

#include <set>

using FilterColumntype = SpaceObjectsManagerMainWindowView::FilterColumnType;
using FilterType = SpaceObjectsManagerMainWindowView::FilterType;
using FilterKeyType = QPair<FilterColumntype, FilterType>;

const QMap<FilterKeyType, QString> SpaceObjectsManagerMainWindowController::kMapFilterStrings {
    {{FilterColumntype::DEBRIS_FILTER, FilterType::ALL_FILTER}, ""},
    {{FilterColumntype::DEBRIS_FILTER, FilterType::YES_FILTER}, "Yes"},
    {{FilterColumntype::DEBRIS_FILTER, FilterType::NO_FILTER}, "No"},
    {{FilterColumntype::LRR_FILTER, FilterType::ALL_FILTER}, ""},
    {{FilterColumntype::LRR_FILTER, FilterType::YES_FILTER}, "Yes"},
    {{FilterColumntype::LRR_FILTER, FilterType::NO_FILTER}, "No"},
    {{FilterColumntype::ILRS_FILTER, FilterType::ALL_FILTER}, ""},
    {{FilterColumntype::ILRS_FILTER, FilterType::YES_FILTER}, "."},
    {{FilterColumntype::ILRS_FILTER, FilterType::NO_FILTER}, "^$"},
    {{FilterColumntype::EN_POLICY_FILTER, FilterType::ALL_FILTER}, ""},
    {{FilterColumntype::EN_POLICY_FILTER, FilterType::YES_FILTER}, "Enabled"},
    {{FilterColumntype::EN_POLICY_FILTER, FilterType::NO_FILTER}, "Disabled"}
};

SpaceObjectsManagerMainWindowController::SpaceObjectsManagerMainWindowController() :
    spaceobjects_view(nullptr)
{

    qRegisterMetaType<QVector<int>>();
    qRegisterMetaType<QItemSelectionModel::SelectionFlags>();
    qRegisterMetaType<QModelIndexList>();
    qRegisterMetaType<QList<QPersistentModelIndex>>();
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>();

    // Load the paths settings.
    this->dir_spaceobjects.setPath(SalaraSettings::instance().getGlobalConfigString("SalaraProjectDataPaths/SP_SpaceObjectsData"));
    this->dir_schemes.setPath(SalaraSettings::instance().getGlobalConfigString("SalaraProjectSchemeFiles/SP_SchemeFiles"));
    this->dir_images.setPath(SalaraSettings::instance().getGlobalConfigString("SalaraProjectDataPaths/SP_SpaceObjectsPictures"));
    this->dir_backup.setPath(SalaraSettings::instance().getGlobalConfigString("SalaraProjectBackupPaths/SP_SpaceObjectsManagerBackup"));
    // Load app settings.
    this->backup_enabled = SalaraSettings::instance().getAppConfigBool("Backup/enabled");
    this->max_backup_files = SalaraSettings::instance().getAppConfigInt("Backup/max_files");

    // Create the model and load the scheme
    this->model = new SpaceObjectModel(this);
    this->slotLoadSpaceObjectsSchemeFile();

    // Create the sort model and assign it to table_view and set the filtering for all columns.
    this->sortmodel = new JsonTableSortFilterProxyModel(this);
    this->sortmodel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    this->sortmodel->setFilterKeyColumn(-1);
    this->sortmodel->setSourceModel(this->model);

    // Hide the rows that not have NORAD (for example when we add a new empty row at object cretion).
    int norad_column = this->model->findColumnSectionByIndex("NORAD");
    this->sortmodel->setFilter(norad_column, ".");

    // Create set model
    this->set_listmodel = new QStringListModel(this);

    // Connect model signals
    this->connectModelSignals();

}

SpaceObjectsManagerMainWindowController::~SpaceObjectsManagerMainWindowController()
{
    qDeleteAll(this->list_sets);
}

void SpaceObjectsManagerMainWindowController::setView(SpaceObjectsManagerMainWindowView *spo_view)
{
    // TODO In the future, it could be allowed to set a new view.
    if (this->spaceobjects_view)
        return;

    SalaraMainWindowController::setView(spo_view);
    spo_view->setModel(this->sortmodel, this->set_listmodel);
    this->spaceobjects_view = spo_view;
    this->connectViewSignals();
    this->slotSelectionChanged();
}

void SpaceObjectsManagerMainWindowController::connectModelSignals()
{
    // Connection for enable the set save button.
    QObject::connect(this->model, &SpaceObjectModel::dataChanged,
    [this](const QModelIndex& tl, const QModelIndex& br, const QVector<int>& roles)
    {
        // TODO
        // Change the role checking if we add more roles in the future.
        int index = this->model->findColumnSectionByIndex("EnablementPolicy");
        if(index >= tl.column() && index <= br.column() && !roles.contains(Qt::EditRole))
        {
            if (this->spaceobjects_view)
                QMetaObject::invokeMethod(this->spaceobjects_view->getTableSelectionModel(), "select",
                                          Qt::AutoConnection, Q_ARG(QModelIndex, this->sortmodel->mapFromSource(tl)),
                                          Q_ARG(QItemSelectionModel::SelectionFlags,
                                                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows));

            // Save set will only be enabled if there is at least one set
            GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveSetsEnabled",
                                       this->set_listmodel->rowCount() > 0);
        }

        this->updateCounterLabels();
    });

    // Connection for change the visible objects label and the set pushbuttons.
    QObject::connect(this->sortmodel, &JsonTableSortFilterProxyModel::rowsInserted, [this]
    {
        GuiLoader::setViewProperty(this->spaceobjects_view, "setVisibleNumber", this->sortmodel->rowCount());
    });
    QObject::connect(this->sortmodel, &JsonTableSortFilterProxyModel::rowsRemoved, [this]
    {
        GuiLoader::setViewProperty(this->spaceobjects_view, "setVisibleNumber", this->sortmodel->rowCount());
    });
    QObject::connect(this->model, &SpaceObjectModel::rowsRemoved,[this]
    {
        GuiLoader::setViewProperty(this->spaceobjects_view, "setNewSetEnabled", this->model->rowCount()!=0);
    });
    QObject::connect(this->model, &SpaceObjectModel::rowsInserted,[this]
    {
        GuiLoader::setViewProperty(this->spaceobjects_view, "setNewSetEnabled", true);
    });
}

void SpaceObjectsManagerMainWindowController::connectViewSignals()
{
    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalEditObject,
                     this, &SpaceObjectsManagerMainWindowController::slotEditObject);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalAddObject,
                     this, &SpaceObjectsManagerMainWindowController::slotAddObject);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalRemoveObjects,
                     this, &SpaceObjectsManagerMainWindowController::slotRemoveObject);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalSaveSpaceObjects,
                     this, &SpaceObjectsManagerMainWindowController::slotSaveSpaceObjectsDataFile);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalNewSet,
                     this, &SpaceObjectsManagerMainWindowController::slotNewSet);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalLoadSet,
                     this, &SpaceObjectsManagerMainWindowController::slotLoadSet);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalSaveSet,
                     this, &SpaceObjectsManagerMainWindowController::slotSaveSet);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalDeleteSet,
                     this, &SpaceObjectsManagerMainWindowController::slotDeleteSet);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalSetSystemSet,
                     this, &SpaceObjectsManagerMainWindowController::slotSetCurrentSystemSet);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalSelectionChanged,
                     this, &SpaceObjectsManagerMainWindowController::slotSelectionChanged);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalLoadSpaceObjects, [this]
    {
        QString filename;
        GuiLoader::exec([this]{
            return QFileDialog::getOpenFileName(this->spaceobjects_view, "Select the json file...",
                                                this->dir_spaceobjects.path(),
                                                "Json Files (*.json *.dat *.txt *.data)");
        }, &filename);
        if(!filename.isEmpty())
            this->slotLoadSpaceObjectsDataFile(filename);
    });
    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalFilterStringChanged,
                     this->sortmodel, &QSortFilterProxyModel::setFilterFixedString);

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalEditEnablementPolicy,
                     [this](const auto& policy)
    {
        QModelIndexList selected_rows_sortmodel = this->spaceobjects_view->getTableSelectionModel()->selectedRows();

        QModelIndexList selected_rows_model;
        for (const auto& row : selected_rows_sortmodel)
        {
            selected_rows_model.append(this->sortmodel->mapToSource(row));
        }

        for (const auto& row : selected_rows_model)
        {
            // Only change enablement policy if it is not always enabled or always disabled
            if (row.data(Qt::UserRole).toInt() != SpaceObject::EnablementPolicy::ALWAYS_ENABLED &&
                row.data(Qt::UserRole).toInt() != SpaceObject::EnablementPolicy::ALWAYS_DISABLED)
            {
                this->model->setData(row, policy, Qt::UserRole);
            }
        }
    });

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalCopyTriggered, this,
                     [this](const QString& column)
    {
        QModelIndex current_index = this->spaceobjects_view->getTableSelectionModel()->currentIndex();
        QModelIndex selected_index = this->sortmodel->index(current_index.row(),
                                                            this->model->findColumnSectionByIndex(column));
        GuiLoader::exec([&selected_index]{QApplication::clipboard()->setText(selected_index.data().toString());});

    });

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalOpenSchemeFile,[this]
    {
        QString scheme_file = this->dir_schemes.path()+"/"+QString(FILE_SPACEOBJECTSSCHEME);
        QDesktopServices::openUrl(QUrl(scheme_file, QUrl::TolerantMode));
    });
    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalOpenDataFile,[this]
    {
        QString config_filename = this->filedata_name;
        QDesktopServices::openUrl(QUrl(config_filename, QUrl::TolerantMode));
    });

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalSetFilter,
                     [this](auto column, auto type)
    {
        auto col_num = this->map_filter_column.find(column);
        auto filter_string = this->kMapFilterStrings.find(qMakePair(column, type));
        if (col_num != this->map_filter_column.end() && filter_string != this->kMapFilterStrings.end())
        {
            if (filter_string->isEmpty())
                this->sortmodel->unsetFilter(this->map_filter_column[column]);
            else
                this->sortmodel->setFilter(this->map_filter_column[column], filter_string.value());
        }
    });

    QObject::connect(this->spaceobjects_view, &SpaceObjectsManagerMainWindowView::signalExportToCSV,
                     this, &SpaceObjectsManagerMainWindowController::exportToCSV);
}

void SpaceObjectsManagerMainWindowController::exportToCSV() const
{
    QModelIndexList list = this->spaceobjects_view->getTableSelectionModel()->selectedRows();

    if(list.empty())
    {
        GuiLoader::exec([view = this->spaceobjects_view]{
        SalaraInformation::showInfo(INFO_SPACEOBJECTMANAGER, "No objects selected.", "", view);});
        return;
    }

    QString filename;

    GuiLoader::exec([view = this->spaceobjects_view]{
        return QFileDialog::getSaveFileName(view, "Export filename.", "", "CSV files (*.csv)");
    }, &filename);

    if(filename.isEmpty())
    {
        GuiLoader::exec([view = this->spaceobjects_view]{
        SalaraInformation::showInfo(INFO_SPACEOBJECTMANAGER, "The filename is empty.", "", view);});
        return;
    }

    if(!filename.contains(".csv"))
        filename+=".csv";

    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);

        stream<<"NORAD;COSPAR;NAME;ILRS;ALTITUDE;RCS;DEBRIS";

        for(const auto& object : list)
        {
            stream<<"\n";
            stream << this->sortmodel->data(this->sortmodel->index(object.row(),1),0).toString() <<";";
            stream << this->sortmodel->data(this->sortmodel->index(object.row(),5),0).toString() <<";";
            stream << this->sortmodel->data(this->sortmodel->index(object.row(),2),0).toString() <<";";
            if(this->sortmodel->data(this->sortmodel->index(object.row(),3),0).toString().isEmpty())
                stream << "No" << ";";
            else
                stream << "Yes" << ";";
            stream << this->sortmodel->data(this->sortmodel->index(object.row(),14),0).toString() <<";";
            stream << this->sortmodel->data(this->sortmodel->index(object.row(),15),0).toDouble() <<";";
            stream << this->sortmodel->data(this->sortmodel->index(object.row(),10),0).toString();
        }

        file.close();
    }
}

void SpaceObjectsManagerMainWindowController::makeDataBackup() const
{
    if (this->backup_enabled)
    {
        QDateTime version_date = QDateTime::currentDateTimeUtc();
        SpaceObjectModel::ExtraParametersMap extraparameters = this->model->getExtraParameters();
        QStringList extraparameters_names(extraparameters.keys());
        QJsonArray extraparameters_array(QJsonArray::fromStringList(extraparameters_names));
        QString backup_file = this->dir_backup.path() + "/SP_SpaceObjectsData_bkp_" +
                version_date.toString("yyyyMMdd_hhmmss") + ".json";
        SpaceObjectFileManager::saveSpaceObjectsData(backup_file, "Space Objects Data Backup " +
                                                     version_date.toString("yyyyMMdd.hhmmss"),
                                                     version_date,
                                                     "This is a Space Objects Data Backup.",
                                                     this->model->getJsonarray(),
                                                     extraparameters_array);

        QDir backup_directory(this->dir_backup.path());
        QStringList backup_files = backup_directory.entryList(QDir::Files, QDir::Time | QDir::Reversed);
        int backup_directory_size = backup_files.size();

        if (backup_directory_size > this->max_backup_files)
        {
            int i = 0;

            while (i < backup_directory_size - this->max_backup_files)
            {
                backup_directory.remove(backup_files[i]);
                i++;
            }
        }
    }
}

void SpaceObjectsManagerMainWindowController::loadSystemSet()
{
    // First disable all that are not always enabled or always disabled
    this->model->disableAll();

    // Check if there is a defined current set and if it exists
    SpaceObjectSet* set = nullptr;
    if (this->system_set.isEmpty() || !(set = this->getSpaceObjectSetByName(this->system_set)))
    {
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentSystemSet", QString());
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentLoadedSet", QString());
    }
    else
    {
        this->loadEnabledObjectsInSet(set);
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentSystemSet", this->system_set);
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentLoadedSet", this->loaded_set);
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentSetIndex",
                                   this->set_listmodel->stringList().indexOf(this->system_set));
    }

    // Update labels.
    this->updateCounterLabels();
}

void SpaceObjectsManagerMainWindowController::loadEnabledObjectsInSet(SpaceObjectSet* set)
{
    int enablement_column = this->model->findColumnSectionByIndex("EnablementPolicy");
    int norad_column = this->model->findColumnSectionByIndex("NORAD");
    // Set enabled objects in set as ENABLED.
    QStringList enabled_objects = set->getEnabled();

    if(enabled_objects.isEmpty())
        return;

    // Filter model by enabled norads. Prepend \\b0* and append \\b to save leading zeros and ensure it is a word.
    JsonTableSortFilterProxyModel aux_sortmodel({{norad_column, "\\b0*" + enabled_objects.join("\\b|\\b0*") + "\\b"}});
    aux_sortmodel.setSourceModel(this->model);
    for (int i = 0; i < aux_sortmodel.rowCount(); i++)
    {
        QModelIndex index = aux_sortmodel.index(i, enablement_column);
        aux_sortmodel.setData(index, SpaceObject::EnablementPolicy::ENABLED, Qt::UserRole);
    }
    // If the number of objects in the sortfilter model is not the same as in the enabled list, then some NORADs do
    // not exist. Find the missing NORADs and display. Then enable save sets button, since set can be changed to fit
    // the new data.
    if (aux_sortmodel.rowCount() != enabled_objects.size())
    {
        for (int i = 0; i < aux_sortmodel.rowCount(); i++)
            enabled_objects.removeAll(aux_sortmodel.data(aux_sortmodel.index(i, norad_column)).toString());

        GuiLoader::exec([&enabled_objects, view = this->spaceobjects_view]{
            SalaraInformation::showInfo(INFO_SPACEOBJECTMANAGER, NORADS_MISSING_MESSAGE, enabled_objects.join('\n'), view);
        });

        GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveSetsEnabled", true);
    }
    // If set is loaded without changes, set save button as disabled
    else
        GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveSetsEnabled", false);

    // TODO At updating model, the selection model was changed programatically by slot connected to datachanged signal
    // from the model. Now the selection must be cleared since it was not done by the user.
    // This could be changed in the future.
    QMetaObject::invokeMethod(this->spaceobjects_view->getTableSelectionModel(), "clear", Qt::AutoConnection);
}

void SpaceObjectsManagerMainWindowController::updateCounterLabels()
{
    // Store the enablement column.
    int enablement_column = this->model->findColumnSectionByIndex("EnablementPolicy");

    // Filter model by enabled objects.
    JsonTableSortFilterProxyModel aux_sortmodel({{enablement_column, kMapFilterStrings[
                                                  {FilterColumntype::EN_POLICY_FILTER, FilterType::YES_FILTER}]}});
    aux_sortmodel.setSourceModel(this->model);

    // Update the show labels.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setLoadedNumber", this->model->rowCount());
    GuiLoader::setViewProperty(this->spaceobjects_view, "setVisibleNumber", this->sortmodel->rowCount());
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabledNumber", aux_sortmodel.rowCount());

}

void SpaceObjectsManagerMainWindowController::slotLoadSpaceObjectsSchemeFile(const QString &path)
{
    // Load Space Objects Model scheme using default file if path is empty
    QString scheme_pathfile = path.isEmpty() ? this->dir_schemes.path()+"/"+QString(FILE_SPACEOBJECTSSCHEME) : path;
    SalaraInformation res = SpaceObjectFileManager::loadSpaceObjectsScheme(scheme_pathfile, *this->model);

    if (res.hasError())
        GuiLoader::exec([&res, view = this->spaceobjects_view]{
            res.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", view);
        });

    else
    {
        // Fill filter columns
        // Debris filter.
        this->map_filter_column[FilterColumntype::DEBRIS_FILTER] = this->model->findColumnSectionByIndex("IsDebris");
        // ILRS filter.
        this->map_filter_column[FilterColumntype::ILRS_FILTER] = this->model->findColumnSectionByIndex("ILRSID");
        // LLR filter.
        this->map_filter_column[FilterColumntype::LRR_FILTER] =
                this->model->findColumnSectionByIndex("LaserRetroReflector");
        // Enablement Policy filter.
        this->map_filter_column[FilterColumntype::EN_POLICY_FILTER] =
                this->model->findColumnSectionByIndex("EnablementPolicy");
    }

}


void SpaceObjectsManagerMainWindowController::slotLoadSet()
{
    int current_index = this->spaceobjects_view->getCurrentSelectedSetIndex();
    // Check that a set is selected.
    if(current_index >= 0)
    {
        const QSignalBlocker blocker(this->spaceobjects_view);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        // Get the set.
        SpaceObjectSet* set = this->list_sets[current_index];
        // First disable all.
        this->spaceobjects_view->setUpdatesEnabled(false);
        this->model->disableAll();
        // Then load enabled objects and update labels
        this->loadEnabledObjectsInSet(set);
        this->updateCounterLabels();
        this->loaded_set = set->getName();
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentLoadedSet", this->loaded_set);
        this->spaceobjects_view->setUpdatesEnabled(true);
        QApplication::restoreOverrideCursor();
    }
}

void SpaceObjectsManagerMainWindowController::slotNewSet()
{
    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    QString name;
    int ok = QDialog::Rejected;

    GuiLoader::exec([this, &name, &ok]{
        QInputDialog dialog(this->spaceobjects_view);
        dialog.setWindowTitle(NAME_SPACEOBJECTMANAGER);
        dialog.setFixedSize(400, 200);
        dialog.setLabelText("Set name:");
        ok = dialog.exec();
        name = dialog.textValue();
    });

    if(ok != QDialog::Accepted)
    {
        // Enables the MainWindow.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
        return;
    }

    if (name.isEmpty())
    {
        GuiLoader::exec([view = this->spaceobjects_view]{
            SalaraInformation::showInfo(INFO_SPACEOBJECTMANAGER, "Empty set name is not valid. Set will not be stored",
                                        "", view);
        });
        // Enables the MainWindow.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
        return;
    }

    // If a set with the same name already exists, show info and return
    if(this->getSpaceObjectSetByName(name))
    {
        GuiLoader::exec([view = this->spaceobjects_view]{
            SalaraInformation::showInfo(NAME_SPACEOBJECTMANAGER, "The set already exists.", "", view);
        });
        // Enables the MainWindow.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
        return;
    }


    // Get the NORADs of every object that is enabled. Always enabled objects are not stored, since this is stored
    // within space objects data
    int norad_column = this->model->findColumnSectionByIndex("NORAD");
    int enablement_column = this->model->findColumnSectionByIndex("EnablementPolicy");
    QStringList norads_enabled;

    for(int i = 0; i<this->model->rowCount(); i++)
    {
        QVariant enablement = this->model->data(this->model->index(i, enablement_column), JsonTableModel::DIRECT_ROLE);
        SpaceObject::EnablementPolicy policy = static_cast<SpaceObject::EnablementPolicy>(enablement.toInt());
        QString norad = this->model->data(this->model->index(i, norad_column), JsonTableModel::DISPLAY_ROLE).toString();
        if(policy == SpaceObject::EnablementPolicy::ENABLED)
            norads_enabled.append(norad);
    }

    // Create a new set with the currently enabled objects
    SpaceObjectSet* set = new SpaceObjectSet(name, norads_enabled);

    // Insert the set data at the list, and the name at the combo box model
    this->set_listmodel->insertRows(this->set_listmodel->rowCount(),1);
    this->set_listmodel->setData(this->set_listmodel->index(this->set_listmodel->rowCount() - 1), name);
    this->list_sets.push_back(set);

    // Save the new set created and set as loaded.
    this->saveSets();

    // Update view
    this->loaded_set = set->getName();
    GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentSetIndex", this->list_sets.size() - 1);
    GuiLoader::setViewProperty(this->spaceobjects_view, "setLoadDeleteSetSystemSetEnabled", true);
    GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentLoadedSet", this->loaded_set);
    GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveSetsEnabled", false);

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}



void SpaceObjectsManagerMainWindowController::slotSaveSpaceObjectsDataFile()
{
    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    int res = QDialog::Rejected;
    QDateTime version_date = QDateTime::currentDateTimeUtc();
    QString name;
    QString version_name;
    QString comment;

    GuiLoader::exec([&, this]{
        FormSave form_save(this->filedata_name, this->filedata_comment, this->filedata_versionname,
                           version_date, this->spaceobjects_view);
        res = form_save.exec();
        name = form_save.getName();
        version_name = form_save.getVersionName();
        comment = form_save.getComment();
    });


    if(res == QDialog::Accepted)
    {

        SpaceObjectModel::ExtraParametersMap extraparameters = this->model->getExtraParameters();
        QStringList extraparameters_names(extraparameters.keys());
        QJsonArray extraparameters_array(QJsonArray::fromStringList(extraparameters_names));

        SalaraInformation errors =
        SpaceObjectFileManager::saveSpaceObjectsData(name, version_name, version_date, comment,
                                                     this->model->getJsonarray(), extraparameters_array);

        if(errors.hasError())
            GuiLoader::exec([view = this->spaceobjects_view, &errors]{
                errors.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", view);
            });
        else
        {
            GuiLoader::exec([&name, view = this->spaceobjects_view]{
                SalaraInformation::showInfo(INFO_SPACEOBJECTMANAGER,
                                            "The space objects data was saved in the json file "
                                            "(check the path by clicking 'Show Details...'.", name, view);
            });
            this->filedata_name = name;
            this->filedata_versionname = version_name;
            this->filedata_versiontime = version_date;
            this->filedata_comment = comment;
            GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveObjectsEnabled", false);
        }
    }

    // Update labels.
    this->updateCounterLabels();

    GuiLoader::setViewProperty(this->spaceobjects_view, "setDBVersionDate", this->filedata_versiontime.toString("yyyy-MM-dd  hh:mm:ss  UTC"));
    GuiLoader::setViewProperty(this->spaceobjects_view, "setDBVersionName", this->filedata_versionname);

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}


void SpaceObjectsManagerMainWindowController::slotLoadSpaceObjectsDataFile(const QString& pathfile)
{
    // TODO is this necessary?
    //this->clearGUI();
    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    // It is empty get the default location stored in the config.
    this->filedata_name = pathfile;
    if(pathfile.isEmpty())
        this->filedata_name = this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSDATA);


    // Loads the model, check for errors and set it in the table view.
    SalaraInformation errors = SpaceObjectFileManager::loadSpaceObjectsData(this->filedata_name, *this->model,
                                                                            this->filedata_comment,
                                                                            this->filedata_versionname,
                                                                            this->filedata_versiontime);
    // Show errors.
    if (errors.hasError())
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        GuiLoader::exec([&errors, view = this->spaceobjects_view]{
            errors.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", view);
        });
        QApplication::restoreOverrideCursor();
    }

    // If there is not an error in scheme, then initialize elements in GUI that depends on it
    if(!errors.containsError(SpaceObjectFileManager::SCHEME_NOT_VALID))
    {
        // Diabled save.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveObjectsEnabled", false);
        GuiLoader::setViewProperty(this->spaceobjects_view, "setNewSetEnabled", this->model->rowCount()>0);

        // Update labels.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setDBVersionDate", this->filedata_versiontime.toString("yyyy-MM-dd  hh:mm:ss  UTC"));
        GuiLoader::setViewProperty(this->spaceobjects_view, "setDBVersionName", this->filedata_versionname);

        // Set enablement policy based on current loaded set, if any
        if (SpaceObjectSet* set = this->getSpaceObjectSetByName(this->loaded_set))
        {
            this->model->disableAll();
            this->loadEnabledObjectsInSet(set);
        }
        this->updateCounterLabels();
    }

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}

void SpaceObjectsManagerMainWindowController::slotDeleteSet()
{
    // If there is no selected set, return
    if (this->spaceobjects_view->getCurrentSelectedSetIndex() < 0)
        return;

    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    // Question for confirmation.
    int selected_set_index = this->spaceobjects_view->getCurrentSelectedSetIndex();
    SpaceObjectSet* selected_set = this->list_sets[selected_set_index];
    QMessageBox::StandardButton reply;

    GuiLoader::exec([this, selected_set]{ return QMessageBox::question(this->spaceobjects_view, INFO_SPACEOBJECTMANAGER,
                                                 DELETE_SET_WARNING(selected_set->getName()));}, &reply);

    // If confirmation is not granted by user, return
    if(reply == QMessageBox::StandardButton::No)
    {
        // Enables the MainWindow.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
        return;
    }

    // Variables.
    QString pathsets = this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSSETS);
    QDateTime version_date = QDateTime::currentDateTimeUtc();

    // Remove the set from model and list
    this->set_listmodel->removeRow(selected_set_index);
    this->list_sets.removeAt(selected_set_index);

    if (!this->list_sets.empty())
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentSetIndex", 0);

    // Set current system set as empty if it is going to be erased.
    if (selected_set->getName() == this->system_set)
    {
        QString pathsets = this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSSETS);
        this->system_set = "";
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentSystemSet", QString());
        SpaceObjectFileManager::saveCurrentSet(this->system_set, pathsets);
    }

    // If loaded set is to be eliminated, then disable all objects.
    if(selected_set->getName() == this->loaded_set)
    {
        this->model->disableAll();
        this->loaded_set = "";
        GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentLoadedSet", QString());
    }

    // Delete the set.
    delete selected_set;

    // Store and save the sets.
    SalaraInformation error = SpaceObjectFileManager::saveSets(pathsets, version_date, this->list_sets);

    // Check for errors.
    if(error.hasError())
        GuiLoader::exec([&error, view = this->spaceobjects_view]{
            error.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", view);
        });

    // Update GUI elements.
    if(this->set_listmodel->rowCount() == 0)
    {
        GuiLoader::setViewProperty(this->spaceobjects_view, "setLoadDeleteSetSystemSetEnabled", false);
        GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveSetsEnabled", false);
    }

    // Update counter labels.
    this->updateCounterLabels();

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}

void SpaceObjectsManagerMainWindowController::slotSaveSet()
{
    // If there is no set selected, return
    if (this->spaceobjects_view->getCurrentSelectedSetIndex() < 0)
        return;

    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    SpaceObjectSet* selected_set = this->list_sets[this->spaceobjects_view->getCurrentSelectedSetIndex()];

    int norad_column = this->model->findColumnSectionByIndex("NORAD");
    int enablement_column = this->model->findColumnSectionByIndex("EnablementPolicy");
    QStringList norads_enabled;

    for(int i = 0; i<this->model->rowCount(); i++)
    {
        QVariant enablement = this->model->data(this->model->index(i, enablement_column), JsonTableModel::DIRECT_ROLE);
        SpaceObject::EnablementPolicy policy = static_cast<SpaceObject::EnablementPolicy>(enablement.toInt());
        QString norad = this->model->data(this->model->index(i, norad_column), JsonTableModel::DISPLAY_ROLE).toString();
        if(policy == SpaceObject::EnablementPolicy::ENABLED)
            norads_enabled.append(norad);
    }

    selected_set->setEnabled(norads_enabled);

    GuiLoader::setViewProperty(this->spaceobjects_view, "setLoadDeleteSetEnabled", true);

    this->saveSets();

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}

void SpaceObjectsManagerMainWindowController::saveSets()
{
    QString pathsets = this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSSETS);
    QDateTime version_date = QDateTime::currentDateTimeUtc();


    // Save the sets.
    SalaraInformation error =  SpaceObjectFileManager::saveSets(pathsets, version_date, this->list_sets);

    // Show info message based on the result of the save operation. If saving is sucessful, save sets button shall be
    // disabled
    GuiLoader::exec([this, &error]{
        if (error.hasError())
            error.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", this->spaceobjects_view);
        else
        {
            SalaraInformation::showInfo(INFO_SPACEOBJECTMANAGER,
                                        "The space objects sets was saved in the json file "
                                        "(check the path by clicking 'Show Details...'.",
                                        this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSSETS),
                                        this->spaceobjects_view);
            this->spaceobjects_view->setSaveSetsEnabled(false);
        }
    });

}

void SpaceObjectsManagerMainWindowController::slotSetCurrentSystemSet()
{
    // If there is no selected set, return
    if (this->spaceobjects_view->getCurrentSelectedSetIndex() < 0)
        return;

    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    // Load selected system set
    this->slotLoadSet();

    QString pathsets = this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSSETS);
    this->system_set = this->list_sets[this->spaceobjects_view->getCurrentSelectedSetIndex()]->getName();

    // Save the sets.
    SalaraInformation error = SpaceObjectFileManager::saveCurrentSet(this->system_set, pathsets);

    // Show info message based on the result of the save operation. If saving is sucessful, save sets button shall be
    // disabled
    GuiLoader::exec([this, &error]{
        if (error.hasError())
            error.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", this->spaceobjects_view);
        else
        {
            SalaraInformation::showInfo(INFO_SPACEOBJECTMANAGER,
                                        "Current Space Objects set was saved in the json file "
                                        "(check the path by clicking 'Show Details...'.",
                                        this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSSETS),
                                        this->spaceobjects_view);
            this->spaceobjects_view->setCurrentSystemSet(this->system_set);
        }
    });

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);

}

SpaceObjectSet *SpaceObjectsManagerMainWindowController::getSpaceObjectSetByName(const QString &name)
{
    auto it = std::find_if(this->list_sets.begin(), this->list_sets.end(),
                           [name](const auto& set){return name == set->getName();});
    return it != this->list_sets.end() ? *it : nullptr;
}


void SpaceObjectsManagerMainWindowController::slotLoadSpaceObjectsSetsFile(const QString &pathfile)
{
    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    // Loads the sets.
    SalaraInformation error =
            SpaceObjectFileManager::loadSpaceObjectsSetsFile(
                pathfile.isEmpty() ? this->dir_spaceobjects.path()+"/"+QString(FILE_SPACEOBJECTSSETS) : pathfile,
                this->list_sets, this->system_set);

    // Check for errors.
    if(error.hasError())
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        GuiLoader::exec([&error, view = this->spaceobjects_view]{
            error.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", view);
        });
        QApplication::restoreOverrideCursor();
    }
    else
    {
        // Set names of loaded sets at model
        QStringList set_names;

        std::transform(this->list_sets.begin(), this->list_sets.end(), std::back_inserter(set_names),
                       [](const auto& set) {return set->getName();});
        this->set_listmodel->setStringList(set_names);
        if (!this->list_sets.empty())
            GuiLoader::setViewProperty(this->spaceobjects_view, "setCurrentSetIndex", 0);
    }

    // Enable/disable the sets buttons.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setLoadDeleteSetSystemSetEnabled", !this->list_sets.isEmpty());

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}

void SpaceObjectsManagerMainWindowController::slotRemoveObject()
{
    // Since selection might be non-contiguous, rows must be removed from the highest to the lowest.
    QModelIndexList selected_indexes = this->spaceobjects_view->getTableSelectionModel()->selectedRows();

    // Prevent removing when there is nothing selected
    if (selected_indexes.empty())
        return;

    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    // Ask user for confirmation before removing object
    QMessageBox::StandardButton reply;
    QString message = "Are you sure to delete the selected objects (" +
            QString::number(selected_indexes.size())+" in total)?";

    GuiLoader::exec([this, &message]{
       return QMessageBox::question(this->spaceobjects_view, INFO_SPACEOBJECTMANAGER, message,
                                    QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                    QMessageBox::StandardButton::No);
    }, &reply);

    if(reply == QMessageBox::No)
    {
        // Enables the MainWindow.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
        return;
    }

    // If all rows are selected, clear the whole model contents
    if (selected_indexes.size() == this->model->rowCount())
    {
        this->model->clearContents();
        // Enables the MainWindow.
        GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
        return;
    }

    // Otherwise, erase elements in reverse order
    // TODO: this fails when deleting a lot of objects if they are selected in ascending order. Possibly a Qt Bug.

    QList<int> rows;

    for (const auto& index : selected_indexes)
        rows.push_back(this->sortmodel->mapToSource(index).row());

    std::sort(rows.begin(), rows.end());

    for (auto i = rows.crbegin(); i != rows.crend(); i++)
        this->model->removeRow(*i);

    GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveObjectsEnabled", true);
    this->updateCounterLabels();

    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}

void SpaceObjectsManagerMainWindowController::slotEditObject()
{
    // Avoid editing without object selected
    if (!this->spaceobjects_view->getTableSelectionModel()->hasSelection())
        return;

    // Disables the MainWindow.
    GuiLoader::exec([this]{this->spaceobjects_view->setEnabled(false);});
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);

    // Get the index of the row to edit and copy its content to the new row
    int source_model_index = this->sortmodel->mapToSource(
                this->spaceobjects_view->getTableSelectionModel()->currentIndex()).row();

    // Creates and exec the form.
    int result = QDialog::Rejected;
    GuiLoader::exec([this, source_model_index]{
        FormSatellite form_spaceobject(this->plugins_search_engines, this->model, source_model_index,
                                       this->spaceobjects_view);
        QApplication::processEvents();
        return form_spaceobject.exec();

    }, &result);

    // Check return code. If code is Accepted, user has confirmed the changes. Otherwise the changes are discarded.
    if(result == QDialog::Accepted)
    {
        // Enable saving, since a row has been edited
        GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveObjectsEnabled", true);
        this->sortmodel->invalidate();
        // Make backup.
        this->makeDataBackup();
    }
    // Reload the selection.
    this->slotSelectionChanged();
    // Restores the MainWindow.
    GuiLoader::exec([this]{this->spaceobjects_view->setEnabled(true);});
}

void SpaceObjectsManagerMainWindowController::slotAddObject()
{
    // Disables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", false);
    // Insert the row.
    this->model->insertRow(this->model->rowCount());

    // Creates and exec the form.
    int result = QDialog::Rejected;
    GuiLoader::exec([this]{
        FormSatellite form_spaceobject(this->plugins_search_engines, this->model, this->model->rowCount() - 1,
                                       this->spaceobjects_view);
        return form_spaceobject.exec();
    }, &result);

    // Check the code.
    if (QDialog::DialogCode::Rejected == result)
    {
        // Erase the row inserted, since it has been rejected
        this->model->removeRow(this->model->rowCount() - 1);
    }
    else
    {
        // Enable save since a new row has been inserted
        GuiLoader::setViewProperty(this->spaceobjects_view, "setSaveObjectsEnabled", true);
        this->sortmodel->invalidate();
        this->makeDataBackup();
        // Map the selection.
        QModelIndex index = this->sortmodel->mapFromSource(this->model->index(this->model->rowCount()-1, 0));
        QMetaObject::invokeMethod(this->spaceobjects_view->getTableSelectionModel(), "select", Qt::AutoConnection,
                                  Q_ARG(QModelIndex, index),
                                  Q_ARG(QItemSelectionModel::SelectionFlags,
                                        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows));
    }
    this->updateCounterLabels();
    // Enables the MainWindow.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnabled", true);
}

void SpaceObjectsManagerMainWindowController::slotSelectionChanged()
{
    // Variables.
    QString image_filename;
    QPixmap pix;
    QImage image;
    QImageReader reader;

    if (!this->spaceobjects_view->getTableSelectionModel())
        return;

    QModelIndexList selected_indexes = this->spaceobjects_view->getTableSelectionModel()->selectedRows();
    int column = this->model->findColumnSectionByIndex("Picture");
    int rows_selected = selected_indexes.size();

    // Update push buttons.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEditSpaceObjectEnabled", 1 == rows_selected);
    GuiLoader::setViewProperty(this->spaceobjects_view, "setRemoveSpaceObjectEnabled", rows_selected > 0);
    GuiLoader::setViewProperty(this->spaceobjects_view, "setEnPolicyOperationsEnabled", rows_selected > 0);

    // Set enabling for copy actions
    GuiLoader::setViewProperty(this->spaceobjects_view, "setCopyActionsEnabled", rows_selected == 1);

    // If only one objects is selected, store the picture path.
    if(rows_selected == 1 &&
            !this->sortmodel->data(sortmodel->index(selected_indexes[0].row(), column)).toString().isEmpty())
    {
        image_filename = this->sortmodel->data(sortmodel->index(selected_indexes[0].row(), column)).toString();
        reader.setFileName(this->dir_images.path()+"/"+image_filename);
        reader.setAutoTransform(true);
        image = reader.read();
        // Check if null.
        pix = image.isNull() ? QPixmap(":/images/default.png") : pix = QPixmap::fromImage(image);
    }
    else
        pix = QPixmap(":/images/default.png");

    // Scale image.
    QSize picture_size = this->spaceobjects_view->getSpaceObjectPictureSize();
    if(pix.height()>pix.width())
        pix = pix.scaledToHeight(picture_size.height(), Qt::SmoothTransformation);
    else
    {
        pix = pix.scaledToWidth(picture_size.width());
        if(pix.size().height() > picture_size.height())
            pix = pix.scaledToHeight(picture_size.height(), Qt::SmoothTransformation);
    }

    // Set pixmap.
    GuiLoader::setViewProperty(this->spaceobjects_view, "setSpaceObjectPicture", pix);
}

void SpaceObjectsManagerMainWindowController::start()
{
    SalaraInformation plugins_errors = this->loadPlugins(
                PluginCategory::EXTERNAL_TOOL | PluginCategory::SPACE_OBJECT_SEARCH_ENGINE);

    if (plugins_errors.hasError())
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        GuiLoader::exec([&plugins_errors, view = this->spaceobjects_view]{
            plugins_errors.showErrors(WARNING_SPACEOBJECTMANAGER, SalaraInformation::WARNING, "", view);
        });
        QApplication::restoreOverrideCursor();
    }

    for (const auto& search_engine : this->plugins.values(PluginCategory::SPACE_OBJECT_SEARCH_ENGINE) )
    {
        this->plugins_search_engines.push_back(qobject_cast<SpaceObjectSearchEngine*>(search_engine));
    }

    this->loadExternalToolsActionsToView();
    this->slotLoadSpaceObjectsDataFile();
    this->slotLoadSpaceObjectsSetsFile();
    this->loaded_set = this->system_set;
    this->loadSystemSet();
}


