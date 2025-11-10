#pragma once

#include <QWidget>
#include <QDialog>

#include "interface_plugin.h"
#include "spcore_global.h"

namespace Ui {
class PluginsSummary;
}

class SP_CORE_EXPORT PluginsSummary : public QDialog
{
    Q_OBJECT

public:
    explicit PluginsSummary(const QList<SPPlugin*>& plugins, QWidget* parent = nullptr);
    ~PluginsSummary();

private:
    Ui::PluginsSummary *ui;
};

