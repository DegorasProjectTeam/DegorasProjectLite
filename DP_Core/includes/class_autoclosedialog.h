#pragma once

#include <QDialog>
#include <QTimer>
#include <QShowEvent>

#include "spcore_global.h"

namespace Ui {
class AutoCloseDialog;
}

class SP_CORE_EXPORT AutoCloseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutoCloseDialog(QWidget *parent = nullptr);

    void setAutoClose(bool);
    void setAutoCloseTime(int);
    void setGroupText(const QString&);
    void setWindowFlags(Qt::WindowFlags);
    void addWidgetToGroup(QWidget *);
    ~AutoCloseDialog() override;

protected:

    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent* event) override;

private:
    Ui::AutoCloseDialog *ui;
    QTimer timer;
    int autoclosetime;
    bool autoclose;
};
