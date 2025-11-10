#include <QMap>
#include <QList>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QPluginLoader>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QDataWidgetMapper>
#include <QItemDelegate>

#include "form_satellite.h"
#include "ui_form_satellite.h"
#include "class_salarasettings.h"
#include "global_texts.h"
#include "class_spaceobjectfilemanager.h"
#include "class_spaceobjectmodel.h"



FormSatellite::FormSatellite(const QList<SpaceObjectSearchEngine*>& plugins, SpaceObjectModel *model, int index,
                             QWidget* parent):
    QDialog(parent),
    ui(new Ui::FormSatellite),
    m_model(model)
{
    // Setup GUI.
    ui->setupUi(this);

    this->installEventFilter(this);

    this->setWindowTitle(QString(NAME_SPACEOBJECTMANAGER)+" - Space Object Form");

    // Setup the validators.
    this->ui->le_norad->setValidator(new QIntValidator(0, 99999, this) );
    this->ui->le_ilrsid->setValidator(new QIntValidator(0, 9999999, this) );

    // Windows modality and others.
    this->setAutoFillBackground(true);
    this->last_norad = "";
    this->ui->le_extra->setReadOnly(true);

    // Save the search engines into combo box and a "Use all engines" option.
    if(!plugins.isEmpty())
    {
        for (const auto& engine : plugins)
        {
            if(engine->isEnabled())
            {
                this->list_plugin.append(engine);
                this->ui->cb_autocomplete->addItem(engine->getPluginName(), QVariant::fromValue(engine));
            }
        }
        if(!this->list_plugin.isEmpty())
        {
            this->ui->cb_autocomplete->insertItem(0, "Use all available engines");
            this->ui->cb_autocomplete->setCurrentIndex(0);
        }
    }
    else
    {
        this->ui->pb_autocomplete->setEnabled(false);
        this->ui->cb_autocomplete->setEnabled(false);
    }


    // Connections.
    QObject::connect(this->ui->pb_clear, &QPushButton::clicked, this, [this]
    {
        this->clearGUI();
        for (auto&& parameter : m_extraparameters_fields)
        {
            parameter->setText("");
        }
        this->m_extraparameters_mapper->submit();
        this->fillExtraParametersField();
    });

    QObject::connect(this->ui->pb_cancel, &QPushButton::clicked, this, &FormSatellite::slotCloseReject);
    QObject::connect(this->ui->pb_autocomplete, &QPushButton::clicked, this, &FormSatellite::searchSpaceObjectData);
    QObject::connect(this->ui->pb_save, &QPushButton::clicked, this, &FormSatellite::checkIntegrity);
    QObject::connect(this->ui->pb_extra, &QPushButton::clicked, [this]
    {
        this->setEnabled(false);
        this->form_extraparameters->setEnabled(true);
        this->form_extraparameters->exec();
        this->setEnabled(true);
    });


    QObject::connect(this->ui->pb_picture, &QPushButton::clicked, [this]
    {

        QString filename = QFileDialog::getOpenFileName(this, tr("Select the object picture..."), "/",
                                                        tr("Image Files (*.png *.jpg *.bmp *.jpeg *.gif)"));
        this->ui->le_picture->setText(filename);
    });

    // Map widgets with model
    m_mapper = new QDataWidgetMapper(this);
    m_mapper->setModel(model);
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    m_mapper->addMapping(this->ui->le_name, model->findColumnSectionByIndex("Name"));
    m_mapper->addMapping(this->ui->sb_altitude, model->findColumnSectionByIndex("Altitude"));
    m_mapper->addMapping(this->ui->sb_binsize, model->findColumnSectionByIndex("BinSize"));
    m_mapper->addMapping(this->ui->sb_inclination, model->findColumnSectionByIndex("Inclination"));
    m_mapper->addMapping(this->ui->sb_npi, model->findColumnSectionByIndex("NormalPointIndicator"));
    m_mapper->addMapping(this->ui->sb_rcs, model->findColumnSectionByIndex("RadarCrossSection"));
    m_mapper->addMapping(this->ui->sb_ampli, model->findColumnSectionByIndex("Amplification"));
    m_mapper->addMapping(this->ui->le_cospar, model->findColumnSectionByIndex("COSPAR"));
    m_mapper->addMapping(this->ui->le_ilrsid, model->findColumnSectionByIndex("ILRSID"));
    m_mapper->addMapping(this->ui->le_ilrsname, model->findColumnSectionByIndex("ILRSName"));
    m_mapper->addMapping(this->ui->le_norad, model->findColumnSectionByIndex("NORAD"));
    m_mapper->addMapping(this->ui->le_picture, model->findColumnSectionByIndex("Picture"));
    m_mapper->addMapping(this->ui->le_sic, model->findColumnSectionByIndex("SIC"));
    m_mapper->addMapping(this->ui->le_laserid, model->findColumnSectionByIndex("LaserID"));
    m_mapper->addMapping(this->ui->cb_cpf, model->findColumnSectionByIndex("ProviderCPF"), "currentText");
    m_mapper->addMapping(this->ui->cb_lrr, model->findColumnSectionByIndex("LaserRetroReflector"));
    m_mapper->addMapping(this->ui->cb_trackpolicy, model->findColumnSectionByIndex("TrackPolicy"));
    m_mapper->addMapping(this->ui->sb_priority, model->findColumnSectionByIndex("Priority"));
    m_mapper->addMapping(this->ui->cb_debris, model->findColumnSectionByIndex("IsDebris"));
    m_mapper->addMapping(this->ui->le_classification, model->findColumnSectionByIndex("Classification"));
    m_mapper->addMapping(this->ui->le_abb, model->findColumnSectionByIndex("Abbreviation"));
    m_mapper->addMapping(this->ui->cb_enablementpolicy, model->findColumnSectionByIndex("EnablementPolicy"));
    m_mapper->setItemDelegate(new SpaceObjectDelegate);

    // TODO: It there are no extraparameters, there is no need to do anything related to them
    m_extraparameters_mapper = new QDataWidgetMapper(this);
    m_extraparameters_mapper->setModel(model);
    m_extraparameters_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    m_extraparameters_mapper->setItemDelegate(new SpaceObjectDelegate);

    // Set the special value text for null variables.
    this->ui->sb_rcs->setSpecialValueText("Null");
    this->ui->sb_npi->setSpecialValueText("Null");
    this->ui->sb_binsize->setSpecialValueText("Null");
    this->ui->sb_altitude->setSpecialValueText("Null");

    // Clear the GUI.
    this->clearGUI();
    // Init the form for editing Extra parameters
    this->initEditExtraParametersDialog();

    // Complete the line edit picture.
    QString dest_folder =
            SalaraSettings::instance().getGlobalConfig()->value("SalaraProjectPaths/SP_SpaceObjectsPictures").toString();

    this->slotSetCurrentSpaceObject(index);

    if(!this->ui->le_picture->text().isEmpty())
        this->ui->le_picture->setText(dest_folder+"/"+this->ui->le_picture->text());
}

void FormSatellite::slotSetCurrentSpaceObject(int index)
{
    this->m_mapper->setCurrentIndex(index);
    this->m_extraparameters_mapper->setCurrentIndex(m_mapper->currentIndex());
    fillExtraParametersField();
}


void FormSatellite::initEditExtraParametersDialog()
{
    QFormLayout *layout = new QFormLayout;
    QVBoxLayout *layout_2 = new QVBoxLayout;
    QHBoxLayout *buttons_layout = new QHBoxLayout;
    buttons_layout->setAlignment(Qt::AlignRight);
    buttons_layout->setSpacing(7);
    QGroupBox* group = new QGroupBox("Extra Parameters");
    group->setLayout(layout);
    group->setAlignment(Qt::AlignCenter);
    this->form_extraparameters = new QDialog(this, Qt::WindowTitleHint);
    this->form_extraparameters->setModal(false);
    this->form_extraparameters->setWindowTitle("");
    this->form_extraparameters->setMinimumWidth(500);

    ExtraParamsEventFilter* filter = new ExtraParamsEventFilter(this);
    this->form_extraparameters->installEventFilter(filter);

    const SpaceObjectModel::ExtraParametersMap &extraparameters = this->m_model->getExtraParameters();

    if(extraparameters.isEmpty())
        this->ui->pb_extra->setEnabled(false);

    for (auto it = extraparameters.cbegin(); it != extraparameters.cend(); it++ )
    {
        QLabel* label = new QLabel(this->form_extraparameters);
        QLineEdit* line = new QLineEdit(this->form_extraparameters);
        line->setClearButtonEnabled(true);
        label->setAccessibleName("label");
        line->setAccessibleName("line");
        label->setText(it.key() + ':');
        this->m_extraparameters_mapper->addMapping(line, it.value());
        layout->addRow(label, line);
        m_extraparameters_fields.insert(it.key(), line);
    }
    QPushButton* save = new QPushButton(this->form_extraparameters);
    QPushButton* cancel = new QPushButton(this->form_extraparameters);
    save->setAccessibleName("Green");
    save->setIcon(this->ui->pb_save->icon());
    cancel->setAccessibleName("Red");
    cancel->setIcon(this->ui->pb_cancel->icon());
    save->setFixedHeight(35);
    save->setFixedWidth(100);
    cancel->setFixedHeight(35);
    cancel->setFixedWidth(100);
    save->setText("  Save");
    cancel->setText("  Cancel");
    save->setCursor(Qt::PointingHandCursor);
    cancel->setCursor(Qt::PointingHandCursor);
    buttons_layout->addWidget(save);
    buttons_layout->addWidget(cancel);
    layout_2->addWidget(group);
    layout_2->addItem(buttons_layout);
    this->form_extraparameters->setLayout(layout_2);


    QObject::connect(save, &QPushButton::clicked, [this]
    {   
        this->m_extraparameters_mapper->submit();
        this->fillExtraParametersField();
        this->form_extraparameters->accept();
    });

    QObject::connect(cancel, &QPushButton::clicked, [this]
    {
        this->m_extraparameters_mapper->revert();
        this->fillExtraParametersField();
        this->form_extraparameters->reject();
    });
}

void FormSatellite::fillExtraParametersField()
{
    QString text = "";
    for (auto it = this->m_extraparameters_fields.begin(); it != this->m_extraparameters_fields.end(); it++)
    {
        text += it.key() + " = " + it.value()->text() + "; ";
    }
    this->ui->le_extra->setText(text);
}

void FormSatellite::searchSpaceObjectData()
{
    // Restore the properties.
    for(auto&& widget : this->findChildren<QWidget*>())
    {
        if(widget->property("error").toBool())
        {
            widget->setProperty("error", false);
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
        }
    }

    // Get the NORAD first.
    QString norad = this->ui->le_norad->text();
    if(norad.isEmpty() || norad.toDouble() == 0.0)
    {
        this->ui->le_norad->setProperty("error", true);
        this->ui->le_norad->setFocus();
        this->ui->le_norad->style()->unpolish(this->ui->le_norad);
        this->ui->le_norad->style()->polish(this->ui->le_norad);
        QMessageBox messagebox(this);
        messagebox.warning(this, WARNING_SPACEOBJECTMANAGER,
                           "A valid NORAD is required to perform the object search.");
        return;
    }

    // Clean GUI if necessary. TODO -> Clean only the params that the engines search.
    if(this->last_norad != norad && !this->last_norad.isEmpty())
    {
        this->m_model->revertEditedRow(this->m_mapper->currentIndex());
        this->m_mapper->revert();
        this->ui->le_norad->setText(norad);
    }

    this->last_norad = norad;

    QList<SpaceObjectSearchEngine*> engines_list;
    QStringList errorlist;
    QStringList infolist;
    QMessageBox messagebox_warning(this);
    QMessageBox messagebox_info(this);
    messagebox_warning.setIcon(QMessageBox::Icon::Warning);
    messagebox_info.setIcon(QMessageBox::Icon::Information);
    messagebox_warning.setWindowTitle(WARNING_SPACEOBJECTMANAGER);
    messagebox_info.setWindowTitle(INFO_SPACEOBJECTMANAGER);

    if(this->ui->cb_autocomplete->currentText()=="Use all available engines")
    {
        // Perform the search using all engines.
        for(int i=0;i<this->ui->cb_autocomplete->count();i++)
            if(!this->ui->cb_autocomplete->itemData(i).isNull())
                engines_list.append(this->ui->cb_autocomplete->itemData(i).value<SpaceObjectSearchEngine*>());
    }
    else
    {
        // Perform the search using the selected engine.
        engines_list.append(this->ui->cb_autocomplete->currentData().value<SpaceObjectSearchEngine*>());
    }

    // Put the app in wait mode.
    this->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    for(const auto& engine : engines_list)
    {
        // Search the space object.

        SpaceObjectSearchEngine::SatelliteSearchEngineError error =
                engine->searchSpaceObjectByNorad(norad, this->object);

        // Check for errors.
        if(error.id==SpaceObjectSearchEngine::ErrorEnum::ConnectionError)
        {
            errorlist.append(engine->getPluginName()+" - Connection Error: "+error.string);
        }
        else if(error.id==SpaceObjectSearchEngine::ErrorEnum::ObjectNotFound)
        {
            infolist.append(engine->getPluginName()+" - Not found (NORAD '"+norad+"').");
        }
        else if(error.id==SpaceObjectSearchEngine::ErrorEnum::ObjectDecayed)
        {
            infolist.append(engine->getPluginName()+" - Decayed (NORAD '"+norad+"').");
            this->clearGUI();
            break;
        }
        else if(error.id==SpaceObjectSearchEngine::ErrorEnum::AltitudeIssue)
        {
            infolist.append(engine->getPluginName()+" - Altitude issue (NORAD '"+norad+"').");
            this->clearGUI();
            break;
        }
        else if(error.id==SpaceObjectSearchEngine::ErrorEnum::NoError)
        {
            autoCompleteFields(this->object);
        }
    }

    QApplication::restoreOverrideCursor();

    if(!errorlist.isEmpty())
    {
        messagebox_warning.setEnabled(true);
        messagebox_warning.setText("An error has occurred. For more details, click 'Show Details...' button.");
        messagebox_warning.setDetailedText(errorlist.join('\n'));
        messagebox_warning.exec();
    }
    if(!infolist.isEmpty())
    {
        messagebox_info.setEnabled(true);
        messagebox_info.setText("A search issue has occurred. For more details, click 'Show Details...' button.");
        messagebox_info.setDetailedText(infolist.join('\n'));
        messagebox_info.exec();
    }

    // Restore the app state.
    this->setEnabled(true);
    QApplication::processEvents();
}


void FormSatellite::autoCompleteFields(const SpaceObject &object)
{
    if(object.getAltitude()>0)
        this->ui->sb_altitude->setValue(object.getAltitude());
    if(object.getBinSize()>0)
        this->ui->sb_binsize->setValue(object.getBinSize());
    if(!qFuzzyCompare(object.getInclination(), -1.0))
        this->ui->sb_inclination->setValue(object.getInclination());
    if(object.getNormalPointIndicator()>0)
        this->ui->sb_npi->setValue(object.getNormalPointIndicator());
    if(object.getRadarCrossSection()>0)
        this->ui->sb_rcs->setValue(object.getRadarCrossSection());
    if(!object.getName().isEmpty())
        this->ui->le_name->setText(object.getName());
    if(!object.getILRSname().isEmpty())
        this->ui->le_ilrsname->setText(object.getILRSname());
    if(!object.getNorad().isEmpty())
        this->ui->le_norad->setText(object.getNorad());
    if(!object.getSIC().isEmpty())
        this->ui->le_sic->setText(object.getSIC());
    if(!object.getCospar().isEmpty())
        this->ui->le_cospar->setText(object.getCospar());
    if(!object.getILRSID().isEmpty())
        this->ui->le_ilrsid->setText(object.getILRSID());
    if(!object.getClassification().isEmpty())
        this->ui->le_classification->setText(object.getClassification());

    if(object.isDebrisChecked())
    {
        if(object.isDebris())
            this->ui->cb_debris->setCurrentText("Yes");
        else
            this->ui->cb_debris->setCurrentText("No");
    }

    if(object.isLaserRetroReflectorChecked())
    {
        if(object.hasLRR())
            this->ui->cb_lrr->setCurrentText("Yes");
        else
            this->ui->cb_lrr->setCurrentText("No");
    }

    if(object.isLaserRetroReflectorChecked() && object.hasLRR())
        this->ui->cb_trackpolicy->setCurrentIndex(this->ui->cb_trackpolicy->findText
                       (SpaceObject::TrackPolicyStringMap[SpaceObject::TrackPolicy::TRACK_ALWAYS]));
    else if (object.isLaserRetroReflectorChecked() && !object.hasLRR())
        this->ui->cb_trackpolicy->setCurrentIndex(this->ui->cb_trackpolicy->findText
                       (SpaceObject::TrackPolicyStringMap[SpaceObject::TrackPolicy::TRACK_ONLY_IF_VISIBLE]));

    this->ui->le_norad->setFocus();
}

void FormSatellite::checkIntegrity()
{
    // Warning messsagebox.
    QMessageBox messagebox(this);
    messagebox.setIcon(QMessageBox::Icon::Warning);
    messagebox.setWindowTitle(WARNING_SPACEOBJECTMANAGER);
    messagebox.setText("Errors have occurred during the processing of the data. "
                       "For more details, click 'Show Details...' button.");

    // Restore the properties.
    for(auto&& widget : this->findChildren<QWidget*>())
    {
        if(widget->property("error").toBool())
        {
            widget->setProperty("error", false);
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
        }
    }

    // Submit the temporal data from the widgets to the model Edit mode.
    m_mapper->submit();

    // Perform an integrity check on the data and submit to the model Display mode
    SpaceObjectModel::ErrorList error_list =
            this->m_model->submitEditedRow(this->m_mapper->currentIndex());

    // If there are no integrity errors, finish this dialog.
    if (error_list.isEmpty())
    {
        // Copy the image in the final folder
        if(!this->ui->le_picture->text().isEmpty())
        {
            QString dest_folder = SalaraSettings::instance().getGlobalConfig()->value(
                        "SalaraProjectPaths/SP_SpaceObjectsPictures").toString();
            QString orig_filepath = this->ui->le_picture->text();
            QFileInfo fileinfo(orig_filepath);
            QString dest_filename = this->ui->le_name->text() + "." + fileinfo.completeSuffix();
            int column = this->m_model->findColumnSectionByIndex("Picture");
            QModelIndex index = this->m_model->index(this->m_mapper->currentIndex(), column);
            this->m_model->setData(index, dest_filename, JsonTableModel::DIRECT_ROLE);
            QString dest_filepath = dest_folder + '/' + dest_filename;
            if(dest_filepath != orig_filepath && QFile::exists(dest_filepath))
            {
                QFile::remove(dest_filepath);
            }
            QFile::copy(orig_filepath, dest_filepath);
        }

        this->accept();
    }
    else
    {
        QString detailed_error;
        for (const auto& error : error_list)
        {
            if(error.first ==SpaceObjectModel::ErrorEnum::MANDATORY_VALUE_EMPTY
                    || error.first == SpaceObjectModel::ErrorEnum::VALUE_NOT_UNIQUE)
            {
                int index = this->m_model->findColumnSectionByIndex(error.second);
                QWidget* widget = m_mapper->mappedWidgetAt(index);
                widget->setProperty("error", true);
                widget->style()->unpolish(widget);
                widget->style()->polish(widget);
            }
            detailed_error += "Index: " + error.second + ". " +
                    SpaceObjectModel::ErrorEnumStringMap[error.first]  + '\n';
        }
        messagebox.setDetailedText(detailed_error);
        messagebox.exec();
    }
}



FormSatellite::~FormSatellite()
{
    delete ui;
}

void FormSatellite::clearGUI()
{
    this->ui->sb_altitude->setValue(0);
    this->ui->sb_binsize->setValue(0);
    this->ui->sb_inclination->setValue(0);
    this->ui->sb_npi->setValue(0);
    this->ui->sb_rcs->setValue(0);
    this->ui->sb_ampli->setValue(0);
    this->ui->sb_priority->setValue(0);
    this->ui->le_cospar->clear();
    this->ui->le_abb->clear();
    this->ui->le_ilrsid->clear();
    this->ui->le_name->clear();
    this->ui->le_ilrsname->clear();
    this->ui->le_norad->clear();
    this->ui->le_picture->clear();
    this->ui->le_sic->clear();
    this->ui->le_laserid->clear();
    this->ui->le_extra->clear();
    this->ui->cb_cpf->setCurrentIndex(0);
    this->ui->le_classification->clear();
    this->ui->cb_debris->setCurrentIndex(2);
    this->ui->cb_lrr->setCurrentIndex(2);
    this->ui->le_norad->setFocus();
    this->ui->cb_trackpolicy->clear();
    for(const auto& s : SpaceObject::TrackPolicyStringMap)
        this->ui->cb_trackpolicy->addItem(s);
    this->ui->cb_enablementpolicy->clear();
    for(const auto& s : SpaceObject::EnablementPolicyStringMap)
        this->ui->cb_enablementpolicy->addItem(s);
    this->ui->cb_cpf->clear();
    this->ui->cb_cpf->addItem(SpaceObject::kAllCPFProvider);
    this->ui->cb_cpf->addItem(SpaceObject::kNoTLEProvider);
}

void SpaceObjectDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (QLineEdit* widget = qobject_cast<QLineEdit*>(editor))
        widget->setText(index.data(Qt::EditRole).toString());
    else if (QComboBox* widget = qobject_cast<QComboBox*>(editor))
        widget->setCurrentText(index.data(Qt::EditRole).toString());
    else if (QSpinBox* widget = qobject_cast<QSpinBox*>(editor))
        widget->setValue(index.data(Qt::EditRole).toString().toInt());
    else if (QDoubleSpinBox* widget = qobject_cast<QDoubleSpinBox*>(editor))
        widget->setValue(index.data(Qt::EditRole).toString().toDouble());
}

void SpaceObjectDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    // TODO: Check if value has changed before setting to model. If value changes but is set to its original value
    // then the delegate should ask the model to discard the changes.
    if (QLineEdit* widget = qobject_cast<QLineEdit*>(editor))
        model->setData(index, widget->text());
    else if (QComboBox* widget = qobject_cast<QComboBox*>(editor))
        model->setData(index, widget->currentIndex());
    else if (QDoubleSpinBox* widget = qobject_cast<QDoubleSpinBox*>(editor))
        model->setData(index, widget->value());
    else if (QSpinBox* widget = qobject_cast<QSpinBox*>(editor))
        model->setData(index, widget->value());
}


bool FormSatellite::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Close)
    {
        this->slotCloseReject();
        event->ignore();
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void FormSatellite::slotCloseReject()
{
    this->m_model->revertEditedRow(this->m_mapper->currentIndex());
    reject();
}
