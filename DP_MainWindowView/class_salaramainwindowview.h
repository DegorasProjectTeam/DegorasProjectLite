#pragma once

#include <QMainWindow>
#include <QFile>

namespace Ui
{
class SalaraMainWindowView;
}

class SalaraMainWindowView : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QString Institution READ institution WRITE setInstitution)
    Q_PROPERTY(QString AppVersion READ appVersion WRITE setAppVersion)
    Q_PROPERTY(QPixmap AppLogo READ appLogo WRITE setAppLogo)

public:
    enum MenuType
    {
        HELP_MENU,
        CONFIG_MENU,
        EDIT_MENU,
        PLUGINS_MENU,
        EXTERNAL_TOOLS_MENU
    };
    enum ActionType
    {
        PLUGINS_SUMMARY,
        ABOUT_PLUGINS,
        OPEN_PLUGINS_FOLDER,
        ABOUT_PROJECT,
        ABOUT_APP,
        CONFIGURE_APP,
        CONFIGURE_PROJECT,
        OPEN_PROJECT_ROOT_FOLDER,
        OPEN_PROJECT_DATA_FOLDER,
        OPEN_PROJECT_CONFIG_FOLDER,
    };

    SalaraMainWindowView(QWidget *parent = nullptr);

    QAction* getAction(ActionType type);
    QMenu* getMenu(MenuType type);

    QWidget* bodyWidget();

    void insertPage(int index, QWidget *page);

    QString institution() const;
    void setInstitution(const QString& institution);

    QString appVersion() const;
    void setAppVersion(const QString& app_version);

    QPixmap appLogo() const;
    void setAppLogo(const QPixmap &app_logo);

    ~SalaraMainWindowView();

public slots:
    void addPage(QWidget *page);
    void setCurrentIndex(int index);

protected:
    void loadDefaultStylesheet();

signals:
    void currentIndexChanged(int index);
    void pageTitleChanged(const QString& title);

private:
    Ui::SalaraMainWindowView* ui;
};

