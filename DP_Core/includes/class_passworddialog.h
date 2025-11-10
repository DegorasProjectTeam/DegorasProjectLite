#pragma once

#include <QDialog>

#include "spcore_global.h"

namespace Ui {
class PasswordDialog;
}

class SP_CORE_EXPORT PasswordDialog : public QDialog
{
    Q_OBJECT

public:

    explicit PasswordDialog(const QString& password = "1234", const QString& app_name = "",
                            const QString& header = "", QWidget *parent = nullptr);

    ~PasswordDialog() override;

    QString getPass() const;

private slots:
    void processPass();

private:
    Ui::PasswordDialog *ui;
    QString password;
    QString app_name;
};
