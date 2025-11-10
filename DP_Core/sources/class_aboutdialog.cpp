#include "includes/class_aboutdialog.h"
#include "ui_form_aboutdialog.h"

#include "includes/class_salarasettings.h"
#include "includes/global_texts.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>


AboutDialog::AboutDialog(const QString& license_file, const QString& about_file,
                         const QIcon& icon, QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Set License text to license text edit
    QFile file(license_file);
    file.open(QFile::ReadOnly);
    QString license (file.readAll());
    file.close();

    this->ui->pte_license->setPlainText(license);

    // Set About text at About section
    QFile about_json_file(about_file);
    about_json_file.open(QIODevice::ReadOnly | QIODevice::Text);

    QString about_json_string = about_json_file.readAll().simplified();
    about_json_file.close();

    QJsonDocument about_jsondocument = QJsonDocument::fromJson(about_json_string.toUtf8());
    QJsonObject about_jsonobject = about_jsondocument["About"].toObject();

    for (auto it = about_jsonobject.constBegin(); it != about_jsonobject.constEnd(); it++ )
    {
        if(it.key()!="DisplayName")
        {
            QLabel* key = new QLabel(it.key(),this);
            QLabel* value = new QLabel(it.value().toString(),this);
            key->setAccessibleName("label");
            value->setAccessibleName("label");
            this->ui->fl_about->addRow(key, value);
        }
        else
            this->setWindowTitle(it.value().toString());
    }

    // Set icon
    this->ui->lb_logo->setPixmap(icon.pixmap(QSize(200,200)));

}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::aboutSalaraProject(QWidget *parent)
{
    AboutDialog about_salara(FILE_SALARAPROJECTLICENSE, FILE_SALARAPROJECTABOUT, QIcon(LOGO_DEGORASPROJECT_V), parent);
    about_salara.setEnabled(true);
    about_salara.exec();

}
