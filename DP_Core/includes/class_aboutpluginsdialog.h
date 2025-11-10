#pragma once

#include "interface_plugin.h"
#include "spcore_global.h"
#include "class_treemodel.h"

#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui{
class FormAboutPlugins;
}

class SP_CORE_EXPORT AboutPluginsDialog : public QDialog
{
    Q_OBJECT
public:
    enum PluginsTreeColumnEnum
    {
        NAME,
        LOADED,
        VERSION,
        AUTHOR,
        ID
    };

    AboutPluginsDialog(const PluginsMultiMap& plugins, QWidget* parent = nullptr);
    ~AboutPluginsDialog() override;

private:

    class AboutPluginsSortFilterProxyModel;

    Ui::FormAboutPlugins *ui;
    PluginsMultiMap m_plugins;
    TreeModel* m_model;
    AboutPluginsSortFilterProxyModel* m_sortmodel;
};

class SP_CORE_EXPORT AboutPluginsDialog::AboutPluginsSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AboutPluginsSortFilterProxyModel(QList<SPPlugin *> plugins, QObject * parent = nullptr);
    ~AboutPluginsSortFilterProxyModel() override = default;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant &value, int role) override;

private:
    QList<SPPlugin*> m_plugins;
};
