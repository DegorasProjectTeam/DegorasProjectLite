
#include "class_mainwindow.h"
#include "class_globalutils.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GlobalUtils::initApp("AlgorithmsTester", "AlgorithmsTester Error",
                      "", "");
    MainWindow window = MainWindow();
    window.resize(900, 600);
    window.show();
    return a.exec();
}

