#include "includes/class_autoclosedialog.h"
#include "ui_form_autoclosedialog.h"

#include "includes/global_texts.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QDialog>
#include <QPainter>
#include <QStyleOption>

AutoCloseDialog::AutoCloseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AutoCloseDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    // Setup time.
    this->autoclose = false;
    this->autoclosetime = 0;

    // Setup layouts.
    this->ui->group->setAlignment(Qt::AlignCenter);

    // Countdown connection.
    QObject::connect(&this->timer, &QTimer::timeout, [this]()
    {
        if(--this->autoclosetime < 0)
        {
            this->timer.stop();
            this->close();
        }
        else
            this->ui->button_close->setText("  Closing this window in "+
                                            QString::number(this->autoclosetime)+" seconds...");
    });

    // Button connection.
    QObject::connect(this->ui->button_close, &QPushButton::clicked, this, &AutoCloseDialog::close);
    QObject::connect(this->ui->button_close, &QPushButton::clicked, &this->timer, &QTimer::stop);
}


void AutoCloseDialog::paintEvent(QPaintEvent *)
{
    if (!(windowFlags() & Qt::FramelessWindowHint) && !testAttribute(Qt::WA_TranslucentBackground))
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Must have style sheet.
    if (testAttribute(Qt::WA_StyleSheetTarget))
    {
        // Let QStylesheetStyle have its way with us.
        QStyleOption opt;
        opt.initFrom(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
        p.end();
        return;
    }
}

void AutoCloseDialog::setAutoClose(bool b)
{
    this->autoclose = b;
}

void AutoCloseDialog::setAutoCloseTime(int t)
{
    this->autoclosetime = t/1000;
}

void AutoCloseDialog::setGroupText(const QString &text)
{
    this->ui->group->setTitle(text);
}

void AutoCloseDialog::setWindowFlags(Qt::WindowFlags flags)
{
    QDialog::setWindowFlags(flags);
}

void AutoCloseDialog::addWidgetToGroup(QWidget *w)
{
    this->ui->group->layout()->addWidget(w);
}

AutoCloseDialog::~AutoCloseDialog()
{
    delete ui;
}

void AutoCloseDialog::showEvent(QShowEvent* event)
{
    if(this->autoclose && this->autoclosetime > 0)
    {
        this->ui->button_close->setText("  Closing this window in "+
                                        QString::number(this->autoclosetime)+" seconds...");
        this->timer.start(1000);
    }
    else
        this->ui->button_close->setText("Close window");

    QWidget::showEvent(event);
}




