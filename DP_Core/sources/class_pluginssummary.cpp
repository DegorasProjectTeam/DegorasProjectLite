#include "includes/class_pluginssummary.h"
#include "includes/global_texts.h"
#include "ui_form_pluginssummary.h"

PluginsSummary::PluginsSummary(const QList<SPPlugin*>& plugins, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginsSummary)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setModal(true);
    this->setWindowTitle(QString(NAME_PROJECT) + " | Plugins Summary");
    QString final;
    QString margin ="style =\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"";
    QString base =
            "<p"+margin+"><span style=\"font-family:'Open Sans Semibold'; font-size:11pt;\">%1</span></p><br>"
            "<p"+margin+"><span style=\"font-family:'Open Sans Semibold'; font-size:10pt;\">Categories: </span>"
                "<span style=\" font-family:'Open Sans'; font-size:10pt;\">%2</span></p><br>"
            "<p"+margin+"><span style=\"font-family:'Open Sans Semibold'; font-size:10pt;\">Author: </span>"
                "<span style=\" font-family:'Open Sans'; font-size:10pt;\">%3</span></p><br>"
            "<p"+margin+"><span style=\"font-family:'Open Sans Semibold'; font-size:10pt;\">Version: </span>"
                "<span style=\" font-family:'Open Sans'; font-size:10pt;\">%4</span></p><br>"
            "<p"+margin+"><span style=\"font-family:'Open Sans Semibold'; font-size:10pt;\">File: </span>"
                "<span style=\" font-family:'Open Sans'; font-size:10pt;text-decoration: underline; color:#55aaff;\">%5</span></p>"
            "<br><br/>";

    for(const auto& plugin : plugins)
    {
        final.append(base.arg(plugin->getPluginName(), pluginCategoryString(plugin),
                     plugin->getPluginCopyright(), plugin->getPluginVersion(), plugin->getPluginDir().path()));
    }
    this->ui->tb_plugins->setHtml(final);
    QObject::connect(this->ui->pb_close, &QPushButton::clicked, this, &PluginsSummary::accept);
}

PluginsSummary::~PluginsSummary()
{
    delete ui;
}
