#include "class_salaramainwindowview.h"
#include "ui_form_salaramainwindowview.h"

#include <QFile>

SalaraMainWindowView::SalaraMainWindowView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SalaraMainWindowView)
{
    ui->setupUi(this);

    // Setup the Project logo with smooth transformation.
    QPixmap logo = this->ui->lb_project_logo->pixmap({});
    this->ui->lb_project_logo->setPixmap(logo.scaledToHeight(this->ui->lb_project_logo->height(),
                                                             Qt::SmoothTransformation));
}

QAction *SalaraMainWindowView::getAction(SalaraMainWindowView::ActionType type)
{
    switch (type)
    {
        case PLUGINS_SUMMARY: return this->ui->actionPluginsSummary;
        case ABOUT_PLUGINS: return this->ui->actionAboutPlugins;
        case OPEN_PLUGINS_FOLDER: return this->ui->actionOpenPluginsFolder;
        case ABOUT_PROJECT: return this->ui->actionAboutSalaraProject;
        case ABOUT_APP: return this->ui->actionAboutApp;
        case CONFIGURE_APP: return this->ui->actionConfigureApp;
        case CONFIGURE_PROJECT: return this->ui->actionConfigureSALARAPROJECT;
        case OPEN_PROJECT_ROOT_FOLDER: return  this->ui->actionOpen_SALARA_PROJECT_root_folder;
        case OPEN_PROJECT_DATA_FOLDER: return this->ui->actionOpenSALARAPROJECTDataFilesFolder;
        case OPEN_PROJECT_CONFIG_FOLDER: return  this->ui->actionOpenSALARAPROJECTConfigFilesFolder;
    }
    return nullptr;
}

QMenu *SalaraMainWindowView::getMenu(SalaraMainWindowView::MenuType type)
{
    switch (type)
    {
        case HELP_MENU: return this->ui->menuHelp;
        case CONFIG_MENU: return this->ui->menuConfiguration;
        case EDIT_MENU: return this->ui->menuEdit;
        case PLUGINS_MENU: return this->ui->menuPlugins;
        case EXTERNAL_TOOLS_MENU: return this->ui->menuExternalTools;
    }
    return nullptr;
}

QWidget *SalaraMainWindowView::bodyWidget()
{
    return this->ui->body_widget->currentWidget();
}

void SalaraMainWindowView::addPage(QWidget* page)
{
    insertPage(this->ui->body_widget->count(), page);
}

void SalaraMainWindowView::setCurrentIndex(int index)
{
    if (index != this->ui->body_widget->currentIndex()) {
        this->ui->body_widget->setCurrentIndex(index);
        emit this->currentIndexChanged(index);
    }
}

void SalaraMainWindowView::loadDefaultStylesheet()
{
    // Apply stylesheet from default stylesheet file to this and central widgets
    QFile stylesheet("://stylesheet_default.qss");
    if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString stylesheet_string = stylesheet.readAll();
        this->setStyleSheet(stylesheet_string);

        for (int i = 0; i < this->ui->body_widget->count(); i++)
        {
            this->ui->body_widget->widget(i)->setStyleSheet(stylesheet_string);
        }
    }

}

void SalaraMainWindowView::insertPage(int index, QWidget *page)
{

    this->ui->body_widget->insertWidget(index, page);

    QString title = page->windowTitle();
    if (title.isEmpty()) {
        page->setWindowTitle("body_widget_page_" + QString::number(index));
    }

}

QString SalaraMainWindowView::institution() const
{
    return this->ui->lb_institution->text();
}

void SalaraMainWindowView::setInstitution(const QString &institution)
{
    this->ui->lb_institution->setText(institution);
}

QString SalaraMainWindowView::appVersion() const
{
    return this->ui->lb_app_version->text();
}

void SalaraMainWindowView::setAppVersion(const QString &app_version)
{
    this->ui->lb_app_version->setText(app_version);
}

QPixmap SalaraMainWindowView::appLogo() const
{
    return this->ui->lb_logo->pixmap({});
}

void SalaraMainWindowView::setAppLogo(const QPixmap& app_logo)
{

    this->ui->lb_logo->setPixmap(app_logo.scaledToHeight(this->ui->lb_logo->height(), Qt::SmoothTransformation));
}

SalaraMainWindowView::~SalaraMainWindowView()
{
    delete ui;
}
