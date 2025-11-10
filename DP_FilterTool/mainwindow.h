#pragma once

#include <QMainWindow>

#include <qwt_plot_barchart.h>
#include "qwt_plot.h"
#include "errorplot.h"
#include "ui_mainwindow.h"


namespace Ui {
class MainWindow;
}

class Plot;
class TrackingData;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

signals:
    void filterChanged();
    void filterSaved();

private slots:
    int threshFilter();

private:
    void closeEvent(QCloseEvent* event) override;

    Ui::MainWindow *ui;
    TrackingData* tracking;
    Plot* d_plot;
    ErrorPlot* error_plot;
    QString file_name;
    bool changed;

};
