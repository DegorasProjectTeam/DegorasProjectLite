
#include <QApplication>
#include <QThread>
#include <QEventLoop>
#include <QTimer>

#include "global_texts.h"
#include "class_globalutils.h"
#include "class_spaceobjectsmanagermainwindowcontroller.h"
#include "class_salarasettings.h"


int main(int argc, char *argv[])
{
    // Application.
    QApplication SP_SpaceObjectsManager(argc, argv);

    GlobalUtils::initApp(NAME_SPACEOBJECTMANAGER, ERROR_SPACEOBJECTMANAGER,
                         NAME_SPACEOBJECTSMANAGERCONFIGFILE, ICON_SPACEOBJECTSMANAGER);


    SpaceObjectsManagerMainWindowController controller;
    SpaceObjectsManagerMainWindowView view;

    QThread controller_thread;
    controller.moveToThread(&controller_thread);
    QObject::connect(&controller_thread, &QThread::finished, &controller_thread, &QThread::deleteLater);
    controller_thread.start();

    view.showMaximized();
    view.setEnabled(false);
    QEventLoop wait_shown;
    QTimer::singleShot(500, &wait_shown, &QEventLoop::quit);
    wait_shown.exec();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    controller.setView(&view);
    controller.start();
    view.setEnabled(true);
    QApplication::restoreOverrideCursor();

    int ret_code = SP_SpaceObjectsManager.exec();

    controller_thread.quit();
    controller_thread.wait(5000);

    return ret_code;
}
