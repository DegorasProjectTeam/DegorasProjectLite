#include "includes/class_passworddialog.h"
#include "includes/class_salarainformation.h"
#include "includes/global_texts.h"

#include "ui_passworddialog.h"

void PasswordDialog::processPass()
{
    if(this->ui->pass_line->text() == this->password)
        emit this->accept();
    else
    {
        SalaraInformation::showError(app_name, PASSWORD_WRONG,"",SalaraInformation::MessageTypeEnum::CRITICAL, this);
    }
}

QString PasswordDialog::getPass() const
{
    return ui->pass_line->text();
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

PasswordDialog::PasswordDialog(const QString &password, const QString &app_name, const QString &header, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog),
    password(password),
    app_name(app_name)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::Tool);
    this->ui->lb_header->setText(header);

    QObject::connect(ui->push_pass, &QDialogButtonBox::accepted, this, &PasswordDialog::processPass);
    QObject::connect(ui->push_pass, &QDialogButtonBox::rejected, this, &PasswordDialog::reject);
    QObject::connect(this, &PasswordDialog::finished, this->ui->pass_line, &QLineEdit::clear);
}
