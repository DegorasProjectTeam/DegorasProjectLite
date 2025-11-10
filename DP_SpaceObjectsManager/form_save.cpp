#include "form_save.h"
#include "ui_form_save.h"

#include <QFileDialog>
#include <QStyle>
#include <QDialog>

#include "global_texts.h"

FormSave::FormSave(QString name, QString comment, QString versionname, const QDateTime& versiontime, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FormSave)
{
    ui->setupUi(this);

    this->setWindowModality(Qt::WindowModal);

    this->setWindowTitle(QString(NAME_SPACEOBJECTMANAGER)+" - Save Form");

    this->name = name;
    this->comment = comment;
    this->version = versionname;
    this->versiontime = versiontime;
    this->ui->dte_date->setDateTime(versiontime);
    this->ui->le_versionname->setText(version);
    this->ui->te_comment->setPlainText(comment);
    this->ui->le_name->setText(name);

    QObject::connect(this->ui->pb_cancel, &QPushButton::clicked, this, &QDialog::reject);

    QObject::connect(this->ui->pb_file, &QPushButton::clicked, [this]
    {
        QString filename = QFileDialog::getSaveFileName(this, tr("Select the json file..."), this->name,
                                                        tr("Json Files (*.json *.dat *.txt *.data)"));
        this->ui->le_name->setText(filename);
    });
    QObject::connect(this->ui->pb_save, &QPushButton::clicked, [this]
    {
        if(this->ui->le_versionname->text().isEmpty())
        {
            this->ui->le_versionname->setProperty("error", true);
            this->ui->le_versionname->style()->unpolish(this->ui->le_versionname);
            this->ui->le_versionname->style()->polish(this->ui->le_versionname);
        }
        else
        {
            this->ui->le_versionname->setProperty("error", false);
            this->ui->le_versionname->style()->unpolish(this->ui->le_versionname);
            this->ui->le_versionname->style()->polish(this->ui->le_versionname);
            this->version = this->ui->le_versionname->text();
        }
        if(this->ui->le_name->text().isEmpty())
        {
            this->ui->le_name->setProperty("error", true);
            this->ui->le_name->style()->unpolish(this->ui->le_name);
            this->ui->le_name->style()->polish(this->ui->le_name);
        }
        else
        {
            this->ui->le_name->setProperty("error", false);
            this->ui->le_name->style()->unpolish(this->ui->le_name);
            this->ui->le_name->style()->polish(this->ui->le_name);
            this->name = this->ui->le_name->text();
        }
        if(this->ui->le_name->text().isEmpty() || this->ui->le_versionname->text().isEmpty())
            QDialog::reject();
        else
        {
            this->comment = this->ui->te_comment->toPlainText();
            QDialog::accept();
        }
    });

    this->ui->pb_save->setFocus();
}

FormSave::~FormSave()
{
    delete ui;
}
