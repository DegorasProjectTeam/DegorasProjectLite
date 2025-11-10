#pragma once

#include <QProgressDialog>
#include <QTimer>

#include "spcore_global.h"

class SP_CORE_EXPORT TimeProgressDialog : public QProgressDialog
{
    Q_OBJECT
public:
    explicit TimeProgressDialog(QWidget *parent = nullptr);
    explicit TimeProgressDialog(const QString& label_text, const QString& cancel_button_text,
                                int min, int max, QWidget *parent = nullptr);
public slots:
    int exec() override;

private:
    QTimer* m_timer;

};
