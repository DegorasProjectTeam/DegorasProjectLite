#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

#include "mainwindow.h"

#include <class_globalutils.h>
#include <global_texts.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    QPalette palette;

    GlobalUtils::initApp("Filter Tool", "Filter Tool",
                         NAME_SPACEOBJECTSMANAGERCONFIGFILE, ICON_SPACEOBJECTSMANAGER);


    // Definimos el estilo visual de la aplicacion.
    a.setStyle(QStyleFactory::create("fusion"));
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::Window, QColor(53,53,53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::ButtonText, Qt::white);
    a.setPalette(palette);

    MainWindow w;
    //w.resize(1000, 500);
    //w.show();
    w.showMaximized();

    return a.exec();
}
