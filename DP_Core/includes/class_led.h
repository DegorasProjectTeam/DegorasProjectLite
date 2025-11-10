#pragma once

#include <QLabel>
#include <QHash>

#include "spcore_global.h"

class QTimer;
class QLabel;

class SP_CORE_EXPORT Led : public QLabel
{
    Q_OBJECT
public:

    enum ColorEnum
    {
        GREEN
    };

    Led(QWidget* parent);
    Led(ColorEnum color = GREEN, QWidget* parent = nullptr);

    void setColor(ColorEnum color);

public slots:
    void setOn();
    void setOff();
    void startBlinking(int period_msec);
    void stopBlinking();

private:
    QTimer* m_timer;
    ColorEnum m_color;
    bool m_led_on;
    static QHash<ColorEnum, QString> m_on_resources;
    static QHash<ColorEnum, QString> m_off_resources;
};

