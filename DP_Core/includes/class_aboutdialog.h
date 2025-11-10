#pragma once

#include <QDialog>

#include "spcore_global.h"

namespace Ui {
class AboutDialog;
}

class SP_CORE_EXPORT AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(const QString &license_file, const QString &about_file,
                         const QIcon &icon, QWidget *parent = nullptr);
    ~AboutDialog();

    static void aboutSalaraProject(QWidget* parent = nullptr);

private:
    Ui::AboutDialog *ui;
};

