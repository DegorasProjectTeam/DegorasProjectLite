#include "includes/class_timeprogressdialog.h"


TimeProgressDialog::TimeProgressDialog(QWidget *parent) :
    TimeProgressDialog("", "Cancel", 0, 0, parent)
{}

TimeProgressDialog::TimeProgressDialog(const QString &label_text,
                                       const QString &cancel_button_text,
                                       int min, int max, QWidget *parent) :
    QProgressDialog (label_text, cancel_button_text, min, max, parent),
    m_timer(new QTimer(this))
{
    this->setValue(min);
    if (max < min)
    {
        this->setMaximum(min);
    }
    m_timer->setInterval(1000);
    connect(this, &TimeProgressDialog::canceled, this, [this]
    {
        this->m_timer->stop();
        this->close();
    });
    connect(m_timer, &QTimer::timeout, this, [this]
    {
        int new_value = this->value() + 1;
        this->setValue(new_value);
        if (new_value == this->maximum())
        {
            this->m_timer->stop();
        }
    });

}

int TimeProgressDialog::exec()
{
    this->m_timer->start();
    return QDialog::exec();
}
