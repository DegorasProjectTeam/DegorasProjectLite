#include "includes/class_spaceobjectdisplaywidget.h"
#include "ui_form_spaceobjectdisplaywidget.h"

#include "includes/class_spaceobjectfilemanager.h"
#include "includes/class_salarasettings.h"
#include "includes/global_texts.h"
#include "includes/class_spaceobjectsmodelloader.h"

#include <QClipboard>
#include <QAction>


SpaceObjectDisplayWidget::SpaceObjectDisplayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpaceObjectDisplayWidget)
{
    ui->setupUi(this);

    // Configure the space objects table.
    this->ui->table_spaceobjects->resizeColumnsToContents();
    this->ui->table_spaceobjects->viewport()->setFocusPolicy(Qt::NoFocus);
    this->ui->table_spaceobjects->setWordWrap(false);
    this->ui->table_spaceobjects->setTabKeyNavigation(true);

    // Hide disabled connection.
    QObject::connect(this->ui->cb_hidedisabled, &QCheckBox::toggled, [this](bool checked)
    {
        int row = this->spaceobject_model->findColumnSectionByIndex("EnablementPolicy");
        if(checked)
            this->spaceobject_sortmodel->setFilter(row, "Enabled");
        else
            this->spaceobject_sortmodel->unsetFilter(row);
        this->ui->label_visible->setText(QString::number(this->spaceobject_sortmodel->rowCount()));
    });

    // Initialize SpaceObjects view
    this->spaceobject_sortmodel = new JsonTableSortFilterProxyModel(this);
    this->spaceobject_sortmodel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    this->ui->table_spaceobjects->setModel(this->spaceobject_sortmodel);
    this->spaceobject_sortmodel->setFilterKeyColumn(-1);
    this->ui->table_spaceobjects->sortByColumn(1, Qt::SortOrder::AscendingOrder);


    // Connection to change the sort column and order.
    QObject::connect(this->ui->table_spaceobjects->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this,
                     [this](int column, Qt::SortOrder order)
    {
        this->ui->table_spaceobjects->sortByColumn(column, order);
    });

    // Connection for setting the search pattern to filter model.
    QObject::connect(this->ui->le_search, &QLineEdit::textChanged,
                     this, [this](const QString& str)
    {
        this->spaceobject_sortmodel->setFilterFixedString(str);
        this->ui->label_visible->setText(QString::number(this->spaceobject_sortmodel->rowCount()));
    });

    // Set connections for copy data
    auto copy_column = [this](const QString& column)
    {
        QModelIndex current_index = this->ui->table_spaceobjects->selectionModel()->currentIndex();
        QModelIndex selected_index = this->spaceobject_sortmodel->index(current_index.row(),
                                                            this->spaceobject_model->findColumnSectionByIndex(column));
        QApplication::clipboard()->setText(selected_index.data().toString());
    };

    // Edit menu actions.
    QAction* action = new QAction(TEXT_ACTION_COPY_NORAD, this);
    action->setShortcut(Qt::CTRL + Qt::Key_N);
    QObject::connect(action, &QAction::triggered, this, [copy_column]{copy_column("NORAD");});
    this->ui->table_spaceobjects->addAction(action);

    action = new QAction(TEXT_ACTION_COPY_COSPAR, this);
    action->setShortcut(Qt::CTRL + Qt::Key_C);
    QObject::connect(action, &QAction::triggered, this, [copy_column]{copy_column("COSPAR");});
    this->ui->table_spaceobjects->addAction(action);

    action = new QAction(TEXT_ACTION_COPY_ILRSID, this);
    QObject::connect(action, &QAction::triggered, this, [copy_column]{copy_column("ILRSID");});
    this->ui->table_spaceobjects->addAction(action);

    action = new QAction(TEXT_ACTION_COPY_NAME, this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_N);
    QObject::connect(action, &QAction::triggered, this, [copy_column]{copy_column("Name");});
    this->ui->table_spaceobjects->addAction(action);

    action = new QAction(TEXT_ACTION_COPY_ILRSNAME, this);
    QObject::connect(action, &QAction::triggered, this, [copy_column]{copy_column("ILRSName");});
    this->ui->table_spaceobjects->addAction(action);

    // Disable actions, since there is no selection
    for (const auto& action : this->ui->table_spaceobjects->actions())
        action->setEnabled(false);

    // Edit actions for space objects table will be enabled if there is an object selected
    QObject::connect(this->ui->table_spaceobjects->selectionModel(), &QItemSelectionModel::selectionChanged,
                     this, [this]
    {
        bool unique = this->ui->table_spaceobjects->selectionModel()->selectedRows().size() == 1;
        for (const auto& action : this->ui->table_spaceobjects->actions())
            action->setEnabled(unique);
    });

    // Add copy actions to context menu of table.
    this->ui->table_spaceobjects->setContextMenuPolicy(Qt::ActionsContextMenu);

    // Finally, load model
    this->loadModel();
}

SpaceObjectDisplayWidget::~SpaceObjectDisplayWidget()
{
    delete ui;
}

void SpaceObjectDisplayWidget::loadModel()
{

    // Configure space objects table
    SalaraInformation load_errors = SpaceObjectsModelLoader::instance().getLoadErrors();
    if (load_errors.hasError())
        // TODO: generic warning
        load_errors.showErrors("Warning", SalaraInformation::WARNING, "", this);

    this->ui->lb_current_set->setText(SpaceObjectsModelLoader::instance().getCurrentSetName());

    this->spaceobject_model = SpaceObjectsModelLoader::instance().getModel();
    this->spaceobject_sortmodel->setSourceModel(this->spaceobject_model);
    this->spaceobject_sortmodel->invalidate();

    // Set size hint for column width
    for (int i = 0; i < this->spaceobject_model->columnCount(); i++)
    {
        int size_hint = this->spaceobject_model->getSizeHintForColumn(i);
        if (0 == size_hint)
            this->ui->table_spaceobjects->hideColumn(i);
        else
            this->ui->table_spaceobjects->setColumnWidth(i, size_hint);
    }

    // Labels.
    this->ui->label_versiondate->setText(
                SpaceObjectsModelLoader::instance().getVersionDate().toString(DATETIME_LABELSTRING));
    this->ui->label_versionname->setText(SpaceObjectsModelLoader::instance().getVersionName());
    this->ui->label_visible->setText(QString::number(this->spaceobject_model->rowCount()));


    // Apply filter for enabled/disabled
    emit this->ui->cb_hidedisabled->toggled(this->ui->cb_hidedisabled->isChecked());

}

QList<QAction*> SpaceObjectDisplayWidget::getEditActions() const
{
    return this->ui->table_spaceobjects->actions();
}

QString SpaceObjectDisplayWidget::getSystemSetName() const
{
    return this->ui->lb_current_set->text();
}

QString SpaceObjectDisplayWidget::getVersionName() const
{
    return this->ui->label_versionname->text();
}

QDateTime SpaceObjectDisplayWidget::getVersionTime() const
{
    return QDateTime::fromString(this->ui->label_versiondate->text(), DATETIME_LABELSTRING);
}


