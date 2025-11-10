#include "includes/class_aboutpluginsdialog.h"

#include "ui_form_pluginsabouts.h"

#include "includes/class_treemodel.h"
#include "includes/global_texts.h"

#include <QDebug>

AboutPluginsDialog::AboutPluginsDialog(const PluginsMultiMap &plugins, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FormAboutPlugins),
    m_plugins(plugins)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setWindowTitle(QString(NAME_PROJECT) + " | About Loaded Plugins");

    QStringList header{"Name", "Loaded", "Version", "Author", "ID"};

    QString model_data;

    int id = 0;
    std::vector<SPPlugin*> plugins_vec;

    for (const auto& category : m_plugins.uniqueKeys())
    {
        // Set string for category at upper level. Enabling depends on children

        QString category_string = pluginCategoryString(category) +
                '\t' + "%1" + '\t' + " " + '\t' + " " + '\t' + QString::number(-1) + '\n';

        int plugins_in_category = 0;
        int plugins_enabled = 0;

        // Set string for every child of the category and get the enabling state of the children
        for (const auto& plugin : m_plugins.values(category))
        {

            // Update plugins enabled counters
            if (plugin->isEnabled())
                plugins_enabled++;

            category_string += "    " + plugin->getPluginName() + '\t' +
                    (plugin->isEnabled() ? "true" : "false") + '\t' +
                    plugin->getPluginVersion() + '\t' +
                    plugin->getPluginCopyright() + '\t' +
                    QString::number(id) + '\n';
            id++;
        }

        // Set the enabling state for the category based on its children enabling state
        QString category_enabled;
        if (0 == plugins_enabled)
        {
            category_enabled = "false";
        }
        else if (plugins_enabled == plugins_in_category)
        {
            category_enabled = "true";
        }
        else
        {
            category_enabled = "partial";
        }

        model_data += category_string.arg(category_enabled);
    }

    m_model = new TreeModel(header, model_data, this);
    m_sortmodel = new AboutPluginsSortFilterProxyModel(plugins.values(), this);
    m_sortmodel->setSourceModel(m_model);

    ui->tv_plugins->setModel(m_sortmodel);
    ui->tv_plugins->expandAll();
    ui->tv_plugins->hideColumn(PluginsTreeColumnEnum::ID);

    for(int i = PluginsTreeColumnEnum::NAME; i <= PluginsTreeColumnEnum::VERSION; i++)
    {
        ui->tv_plugins->resizeColumnToContents(i);
    }

}

AboutPluginsDialog::~AboutPluginsDialog()
{
    delete ui;
}

AboutPluginsDialog::AboutPluginsSortFilterProxyModel::AboutPluginsSortFilterProxyModel(QList<SPPlugin *> plugins, QObject *parent) :
    QSortFilterProxyModel (parent),
    m_plugins(std::move(plugins))
{

}

Qt::ItemFlags AboutPluginsDialog::AboutPluginsSortFilterProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    else if (AboutPluginsDialog::LOADED == index.column() && index.parent().isValid())
    {
        Qt::ItemFlags check_flags = Qt::ItemIsUserCheckable | QAbstractProxyModel::flags(index);
        check_flags = static_cast<Qt::ItemFlags>(Qt::ItemIsEditable ^ check_flags);

        return  check_flags;
    }
    else
    {
        return static_cast<Qt::ItemFlags>(Qt::ItemIsEditable ^ QAbstractProxyModel::flags(index));
    }

}

QVariant AboutPluginsDialog::AboutPluginsSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && AboutPluginsDialog::LOADED == index.column())
    {
        if (Qt::CheckStateRole == role)
        {
            QString state = QSortFilterProxyModel::data(index).toString();

            if (state == "true")
            {
                return Qt::Checked;
            }
            else if (state == "false")
            {
                return Qt::Unchecked;
            }
            else
            {
                return Qt::PartiallyChecked;
            }
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        return QSortFilterProxyModel::data(index, role);
    }
}

bool AboutPluginsDialog::AboutPluginsSortFilterProxyModel::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid())
    {
        if (AboutPluginsDialog::LOADED == index.column() && role == Qt::CheckStateRole)
        {
            // Get the TreeItem and find if there is an ID for the plugin
            // The internal pointer must be obtained from original model, not from proxy model
            QModelIndex source_index = mapToSource(index);
            TreeItem * item = static_cast<TreeItem*>(source_index.internalPointer());
            int id = item->data(AboutPluginsDialog::ID).toString().toInt();
            SPPlugin* plugin = nullptr;

            // TODO: handle category parent check state change
            if (id >= 0 && id < this->m_plugins.size() && (plugin = m_plugins[id]))
            {

                QString string_value;
                if (value == Qt::Checked)
                {
                    string_value = "true";
                }
                else if (value == Qt::Unchecked)
                {
                    string_value = "false";
                }
                else
                {
                    string_value = "partial";
                }

                // Set enabling to plugin
                plugin->setEnabled(value == Qt::Checked);

                // Store the enabling state in the model and emit datachanged signal
                item->setData(AboutPluginsDialog::LOADED, string_value);
                emit dataChanged(index, index, {Qt::CheckStateRole});

                // Update enabling of parent based on its children enabling state
                TreeItem * parent = item->parent();
                if (parent)
                {
                    int plugins_by_category = 0;
                    int plugins_enabled_by_category = 0;

                    for (const auto& child : parent->children())
                    {
                        plugins_by_category++;
                        if ("true" == child->data(AboutPluginsDialog::LOADED).toString())
                        {
                            plugins_enabled_by_category++;
                        }
                    }

                    if (plugins_enabled_by_category == plugins_by_category)
                    {
                        parent->setData(AboutPluginsDialog::LOADED,"true");
                    }
                    else if (0 == plugins_enabled_by_category)
                    {
                        parent->setData(AboutPluginsDialog::LOADED, "false");
                    }
                    else
                    {
                        parent->setData(AboutPluginsDialog::LOADED, "partial");
                    }

                    QModelIndex parent_in_same_column =
                            this->index(index.parent().row(), AboutPluginsDialog::LOADED, index.parent().parent());
                    emit dataChanged(parent_in_same_column, parent_in_same_column, {Qt::CheckStateRole});
                }

                return true;
            }
            // Otherwise return false
            else
            {
                return false;
            }
        }
        else
        {
            return QSortFilterProxyModel::setData(index, value, role);
        }
    }
    else
    {
        return false;
    }
}
