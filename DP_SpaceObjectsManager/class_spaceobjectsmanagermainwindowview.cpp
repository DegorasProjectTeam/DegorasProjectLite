#include "class_spaceobjectsmanagermainwindowview.h"
#include "ui_spaceobjectsmanagermainwindowview.h"
#include "form_satellite.h"
#include "form_save.h"

#include <class_spaceobject.h>
#include <class_spaceobjectmodel.h>
#include <global_texts.h>

#include <QMessageBox>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QInputDialog>
#include <QAbstractProxyModel>
#include <QWidgetAction>


//QWidgetAction* createTextSeparator(const QString& text)
//{
//auto* pLabel = new QLabel(text);
//// grayish style
//pLabel->setStyleSheet("background: #FF4B4B4B;");
//// possible alignment
//// pLabel->setAlignment(Qt::AlignCenter);
//auto* separator = new QWidgetAction(this);
//separator->setDefaultWidget(pLabel);
//return separator;
//}

SpaceObjectsManagerMainWindowView::SpaceObjectsManagerMainWindowView(QWidget *parent) :
    SalaraMainWindowView(parent),
    m_ui(new Ui::SpaceObjectsManagerMainWindowView)
{
    this->m_ui->setupUi(this);
    this->loadDefaultStylesheet();
    this->clearGUI();

    this->setWindowTitle(QString(NAME_SPACEOBJECTMANAGER));


    this->menu_data_ = this->menuBar()->addMenu("Data");
    this->menu_data_->addSeparator()->setText("Objects Data");
    this->menu_data_->setSeparatorsCollapsible(true);
    this->menu_export_ = this->menu_data_->addMenu("Export");

    this->export_csv_ = this->menu_export_->addAction("Export to CSV", this,
                                                      &SpaceObjectsManagerMainWindowView::signalExportToCSV);

    makeConnections();
}


SpaceObjectsManagerMainWindowView::~SpaceObjectsManagerMainWindowView()
{
    delete m_ui;
}

void SpaceObjectsManagerMainWindowView::setModel(QAbstractItemModel* table_model, QAbstractItemModel* sets_model)
{
    this->m_ui->table_view->setModel(table_model);
    this->m_ui->table_view->sortByColumn(1, Qt::SortOrder::AscendingOrder);

    // Set size hint for column width if space objects model is set
    SpaceObjectModel* spaceobject_model;
    if (QAbstractProxyModel* proxy_model = qobject_cast<QAbstractProxyModel*>(table_model))
        spaceobject_model = qobject_cast<SpaceObjectModel*>(proxy_model->sourceModel());
    else
        spaceobject_model = qobject_cast<SpaceObjectModel*>(table_model);

    if (spaceobject_model)
    {
        for (int i = 0; i < spaceobject_model->columnCount(); i++)
        {
            int size_hint = spaceobject_model->getSizeHintForColumn(i);
            if (0 == size_hint)
                this->m_ui->table_view->hideColumn(i);
            else
                this->m_ui->table_view->setColumnWidth(i, size_hint);
        }
    }

    this->m_ui->cb_set->setModel(sets_model);
    if (sets_model->rowCount() > 0)
        this->m_ui->cb_set->setCurrentIndex(0);

    QObject::connect(this->m_ui->table_view->selectionModel(), &QItemSelectionModel::selectionChanged,
                     this, &SpaceObjectsManagerMainWindowView::signalSelectionChanged);

    QObject::connect(table_model, &QAbstractItemModel::rowsAboutToBeRemoved,
                     this->m_ui->table_view->selectionModel(), &QItemSelectionModel::clear);

}

QItemSelectionModel *SpaceObjectsManagerMainWindowView::getTableSelectionModel()
{
    return this->m_ui->table_view->selectionModel();
}

int SpaceObjectsManagerMainWindowView::getCurrentSelectedSetIndex() const
{
    return this->m_ui->cb_set->currentIndex();
}

QSize SpaceObjectsManagerMainWindowView::getSpaceObjectPictureSize() const
{
    return this->m_ui->label_picture->size();
}


//
// ========== EVENT FILTERS ============================================================================================
//
void SpaceObjectsManagerMainWindowView::closeEvent(QCloseEvent *event)
{
    if (this->m_ui->pb_save->isEnabled())
    {
        QString message = tr("There are unsaved changes in space objects.\nQuit without saving anyway?\n");
        QMessageBox::StandardButton resBtn =
                QMessageBox::question( this, NAME_SPACEOBJECTMANAGER, message,
                                       QMessageBox::No | QMessageBox::Yes,
                                       QMessageBox::No);
        if (resBtn != QMessageBox::Yes)
        {
            event->ignore();
        }
        else
        {
            event->accept();
        }
    }

    if (this->m_ui->pb_saveset->isEnabled())
    {
        QString message = tr("There are unsaved changes in sets.\nQuit without saving anyway?\n");
        QMessageBox::StandardButton resBtn =
                QMessageBox::question( this, NAME_SPACEOBJECTMANAGER, message,
                                       QMessageBox::No | QMessageBox::Yes,
                                       QMessageBox::No);
        if (resBtn != QMessageBox::Yes)
        {
            event->ignore();
        }
        else
        {
            event->accept();
        }
    }
}

void SpaceObjectsManagerMainWindowView::resizeEvent(QResizeEvent* event)
{
    bool flag = event->size().width()>1500;
    this->m_ui->label_versiondate_title->setVisible(flag);
    this->m_ui->label_versionname_title->setVisible(flag);
    this->m_ui->label_versiondate->setVisible(flag);
    this->m_ui->label_versionname->setVisible(flag);
    if (flag)
    {
        this->m_ui->lb_enpolicylabel->setText("Enablement Policy:");
        this->m_ui->lb_lrrlabel->setText("Laser Retro Reflector:");
        this->m_ui->rb_ilrsis->setText("Is");
        this->m_ui->rb_ilrsisnot->setText("Is Not");
        this->m_ui->rb_ilrsall->setText("All");
        this->m_ui->rb_lrrwith->setText("With");
        this->m_ui->rb_lrrwithout->setText("Without");
        this->m_ui->rb_lrrall->setText("All");
        this->m_ui->rb_debrisis->setText("Is");
        this->m_ui->rb_debrisisnot->setText("Is not");
        this->m_ui->rb_debrisall->setText("All");
        this->m_ui->rb_enpolicyenabled->setText("Enabled");
        this->m_ui->rb_enpolicydisabled->setText("Disabled");
        this->m_ui->rb_enpolicyall->setText("All");
    }
    else
    {
        this->m_ui->lb_enpolicylabel->setText("En.Pol.:");
        this->m_ui->lb_lrrlabel->setText("LRR:");
        this->m_ui->rb_ilrsis->setText("Y");
        this->m_ui->rb_ilrsisnot->setText("N");
        this->m_ui->rb_ilrsall->setText("A");
        this->m_ui->rb_lrrwith->setText("Y");
        this->m_ui->rb_lrrwithout->setText("N");
        this->m_ui->rb_lrrall->setText("A");
        this->m_ui->rb_debrisis->setText("Y");
        this->m_ui->rb_debrisisnot->setText("N");
        this->m_ui->rb_debrisall->setText("A");
        this->m_ui->rb_enpolicyenabled->setText("E");
        this->m_ui->rb_enpolicydisabled->setText("D");
        this->m_ui->rb_enpolicyall->setText("A");
    }
    SalaraMainWindowView::resizeEvent(event);
    emit this->signalSelectionChanged();
}

void SpaceObjectsManagerMainWindowView::keyPressEvent(QKeyEvent *event)
{
    if(this->m_ui->table_view->selectionModel()->selectedRows().size() == 1)
    {
        if(event->key()==Qt::Key_Down)
        {
            QModelIndex index = this->m_ui->table_view->selectionModel()->currentIndex();
            if(index.row()+1 < this->m_ui->table_view->model()->rowCount())
                this->m_ui->table_view->selectRow(index.row()+1);
        }
        else if(event->key()==Qt::Key_Up)
        {
            QModelIndex index = this->m_ui->table_view->selectionModel()->currentIndex();
            if(index.row()-1 >= 0)
                this->m_ui->table_view->selectRow(index.row()-1);
        }
        else if(event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return)
        {
            emit this->signalEditObject();
        }
    }
    if(this->m_ui->table_view->selectionModel()->selectedRows().size() >= 1)
    {
        if(event->key()==Qt::Key_Delete)
        {
            emit this->signalRemoveObjects();
        }
    }
    QMainWindow::keyPressEvent(event);
}
//======================================================================================================================

void SpaceObjectsManagerMainWindowView::clearGUI()
{
    // Sets Edit and Remove buttons to disabled since there is no selection
    this->m_ui->pb_editspaceobject->setEnabled(false);
    this->m_ui->pb_removespaceobject->setEnabled(false);
    this->m_ui->pb_editenablementpolicy->setEnabled(false);
    this->m_ui->pb_newset->setEnabled(false);
    this->m_ui->pb_saveset->setEnabled(false);
    this->m_ui->pb_deleteset->setEnabled(false);
    this->m_ui->pb_set_current_set->setEnabled(false);
    this->m_ui->pb_loadset->setEnabled(false);
    this->m_ui->pb_save->setEnabled(false);
    this->m_ui->cb_set->clear();
    // Fill Enablement Policy combobox
    this->m_ui->cb_enablementpolicy->clear();
    this->m_ui->cb_enablementpolicy->addItem(
                SpaceObject::EnablementPolicyStringMap[SpaceObject::EnablementPolicy::DISABLED]);
    this->m_ui->cb_enablementpolicy->addItem(
                SpaceObject::EnablementPolicyStringMap[SpaceObject::EnablementPolicy::ENABLED]);
    // Clear labels.
    this->m_ui->label_loaded->setText("");
    this->m_ui->label_visible->setText("");
    this->m_ui->label_versionname->setText("");
    this->m_ui->label_versiondate->setText("");
    this->m_ui->lb_loaded_set->setText("");
    this->m_ui->lb_current_set->setText("");

    // Configure the table view.
    this->m_ui->table_view->resizeColumnsToContents();
    this->m_ui->table_view->viewport()->setFocusPolicy(Qt::NoFocus);
    this->m_ui->table_view->setWordWrap(false);
    this->m_ui->table_view->setTabKeyNavigation(true);
    this->m_ui->table_view->verticalHeader()->setVisible(false);

    // Set filters to All by default
    this->m_ui->rb_lrrall->toggle();
    this->m_ui->rb_debrisall->toggle();
    this->m_ui->rb_enpolicyall->toggle();
    this->m_ui->rb_ilrsall->toggle();
}

void SpaceObjectsManagerMainWindowView::setCurrentSetIndex(int index)
{
    this->m_ui->cb_set->setCurrentIndex(index);
}

void SpaceObjectsManagerMainWindowView::setLoadedNumber(int num)
{
    this->m_ui->label_loaded->setText(QString::number(num));
}

void SpaceObjectsManagerMainWindowView::setVisibleNumber(int num)
{
    this->m_ui->label_visible->setText(QString::number(num));
}

void SpaceObjectsManagerMainWindowView::setDBVersionName(const QString& version)
{
    this->m_ui->label_versionname->setText(version);
}

void SpaceObjectsManagerMainWindowView::setDBVersionDate(const QString& date)
{
    this->m_ui->label_versiondate->setText(date);
}

void SpaceObjectsManagerMainWindowView::setEnabledNumber(int num)
{
    this->m_ui->label_enabled->setText(QString::number(num));
}

void SpaceObjectsManagerMainWindowView::setSaveObjectsEnabled(bool enabled)
{
    this->m_ui->pb_save->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setSaveSetsEnabled(bool enabled)
{
    this->m_ui->pb_saveset->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setCurrentSystemSet(const QString &set_name)
{
    this->m_ui->lb_current_set->setText(set_name);
}

void SpaceObjectsManagerMainWindowView::setCurrentLoadedSet(const QString &set_name)
{
    this->m_ui->lb_loaded_set->setText(set_name);
}

void SpaceObjectsManagerMainWindowView::setNewSetEnabled(bool enabled)
{
    this->m_ui->pb_newset->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setLoadDeleteSetSystemSetEnabled(bool enabled)
{
    this->m_ui->pb_set_current_set->setEnabled(enabled);
    this->setLoadDeleteSetEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setLoadDeleteSetEnabled(bool enabled)
{
    this->m_ui->pb_loadset->setEnabled(enabled);
    this->m_ui->pb_deleteset->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setCopyActionsEnabled(bool enabled)
{
    QMenu* edit_menu = this->getMenu(SalaraMainWindowView::MenuType::EDIT_MENU);

    if (edit_menu)
        for(auto&& action : edit_menu->actions())
            action->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setEnPolicyOperationsEnabled(bool enabled)
{
    this->m_ui->pb_editenablementpolicy->setEnabled(enabled);
    this->m_ui->cb_enablementpolicy->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setEditSpaceObjectEnabled(bool enabled)
{
    this->m_ui->pb_editspaceobject->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setRemoveSpaceObjectEnabled(bool enabled)
{
    this->m_ui->pb_removespaceobject->setEnabled(enabled);
}

void SpaceObjectsManagerMainWindowView::setSpaceObjectPicture(const QPixmap &pixmap)
{
    this->m_ui->label_picture->setPixmap(pixmap);
}

int SpaceObjectsManagerMainWindowView::execFormSatellite(const QList<SpaceObjectSearchEngine *> &plugins,
                                                         SpaceObjectModel *model, int index)
{
    FormSatellite form_spaceobject(plugins, model, index, this);
    return form_spaceobject.exec();
}

void SpaceObjectsManagerMainWindowView::makeConnections()
{

    // Slot for edit the enablement policy quickly.
    QObject::connect(this->m_ui->pb_editenablementpolicy, &QPushButton::clicked, [this]
    {
        SpaceObject::EnablementPolicy policy =
                SpaceObject::EnablementPolicyEnumMap[this->m_ui->cb_enablementpolicy->currentText()];
        emit this->signalEditEnablementPolicy(policy);
    });


    // Trivial controller request connections
    QObject::connect(this->m_ui->pb_editspaceobject, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalEditObject);
    QObject::connect(this->m_ui->table_view, &QTableView::doubleClicked,
                     this, &SpaceObjectsManagerMainWindowView::signalEditObject);
    QObject::connect(this->m_ui->pb_addspaceobject, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalAddObject);
    QObject::connect(this->m_ui->pb_removespaceobject, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalRemoveObjects);
    QObject::connect(this->m_ui->pb_save, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalSaveSpaceObjects);
    QObject::connect(this->m_ui->pb_newset, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalNewSet);
    QObject::connect(this->m_ui->pb_loadset, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalLoadSet);
    QObject::connect(this->m_ui->pb_saveset, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalSaveSet);
    QObject::connect(this->m_ui->pb_deleteset, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalDeleteSet);
    QObject::connect(this->m_ui->pb_set_current_set, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalSetSystemSet);
    QObject::connect(this->m_ui->pb_loadobjectmaster, &QPushButton::clicked,
                     this, &SpaceObjectsManagerMainWindowView::signalLoadSpaceObjects);
    QObject::connect(this->m_ui->le_search, &QLineEdit::textChanged,
                     this, &SpaceObjectsManagerMainWindowView::signalFilterStringChanged);

    // Trivial view connections
    QObject::connect(this->m_ui->pb_selectall, &QPushButton::clicked, this->m_ui->table_view, &QTableView::selectAll);
    QObject::connect(this->m_ui->pb_deselectall, &QPushButton::clicked, this->m_ui->table_view, &QTableView::clearSelection);

    // Connection to change the sort column and order.
    QObject::connect(this->m_ui->table_view->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
                     [this](int column, Qt::SortOrder order) {this->m_ui->table_view->sortByColumn(column, order);});


    // Actions.
    QObject::connect(this->m_ui->actionCopyNorad, &QAction::triggered, [this]{emit this->signalCopyTriggered("NORAD");});
    QObject::connect(this->m_ui->actionCopyCOSPAR, &QAction::triggered, [this]{emit this->signalCopyTriggered("COSPAR");});
    QObject::connect(this->m_ui->actionCopyILRSID, &QAction::triggered, [this]{emit this->signalCopyTriggered("ILRSID");});
    QObject::connect(this->m_ui->actionCopyName, &QAction::triggered, [this]{emit this->signalCopyTriggered("Name");});
    QObject::connect(this->m_ui->actionCopyILRSName, &QAction::triggered, [this]{emit this->signalCopyTriggered("ILRSName");});

    QObject::connect(this->m_ui->actionOpenSchemeFile, &QAction::triggered,
                     this, &SpaceObjectsManagerMainWindowView::signalOpenSchemeFile);
    QObject::connect(this->m_ui->actionOpenCurrentData, &QAction::triggered,
                     this, &SpaceObjectsManagerMainWindowView::signalOpenDataFile);


    // Add copy actions to context menu of table and to Edit menu
    QList<QAction*> actions;
    actions << this->m_ui->actionCopyNorad << this->m_ui->actionCopyCOSPAR
            << this->m_ui->actionCopyILRSID << this->m_ui->actionCopyName << this->m_ui->actionCopyILRSName;
    this->m_ui->table_view->setContextMenuPolicy(Qt::ActionsContextMenu);
    this->m_ui->table_view->addActions(actions);
    QMenu* edit_menu = this->getMenu(SalaraMainWindowView::MenuType::EDIT_MENU);
    if (edit_menu)
        edit_menu->addActions(actions);

    // Add open actions to Configure menu
    QMenu* config_menu = this->getMenu(SalaraMainWindowView::MenuType::CONFIG_MENU);
    if (config_menu)
    {
        config_menu->addSeparator();
        config_menu->addAction(this->m_ui->actionOpenSchemeFile);
        config_menu->addAction(this->m_ui->actionOpenCurrentData);
    }

    // Debris filter
    QObject::connect(this->m_ui->rb_debrisall, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::DEBRIS_FILTER, FilterType::ALL_FILTER);});
    QObject::connect(this->m_ui->rb_debrisis, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::DEBRIS_FILTER, FilterType::YES_FILTER);});
    QObject::connect(this->m_ui->rb_debrisisnot, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::DEBRIS_FILTER, FilterType::NO_FILTER);});
    // ILRS filter.
    QObject::connect(this->m_ui->rb_ilrsall, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::ILRS_FILTER, FilterType::ALL_FILTER);});
    QObject::connect(this->m_ui->rb_ilrsis, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::ILRS_FILTER, FilterType::YES_FILTER);});
    QObject::connect(this->m_ui->rb_ilrsisnot, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::ILRS_FILTER, FilterType::NO_FILTER);});
    // LLR filter.
    QObject::connect(this->m_ui->rb_lrrall, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::LRR_FILTER, FilterType::ALL_FILTER);});
    QObject::connect(this->m_ui->rb_lrrwith, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::LRR_FILTER, FilterType::YES_FILTER);});
    QObject::connect(this->m_ui->rb_lrrwithout, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::LRR_FILTER, FilterType::NO_FILTER);});
    // Enablement Policy filter.
    QObject::connect(this->m_ui->rb_enpolicyall, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::EN_POLICY_FILTER, FilterType::ALL_FILTER);});
    QObject::connect(this->m_ui->rb_enpolicyenabled, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::EN_POLICY_FILTER, FilterType::YES_FILTER);});
    QObject::connect(this->m_ui->rb_enpolicydisabled, &QRadioButton::clicked, [this]
    { emit this->signalSetFilter(FilterColumnType::EN_POLICY_FILTER, FilterType::NO_FILTER);});

}
