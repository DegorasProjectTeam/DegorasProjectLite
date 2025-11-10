#pragma once

#include <QPen>
#include <qwt_plot.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_curve.h>


#include <QDebug>
#include <QMouseEvent>
#include <QObject>
#include <QEvent>
#include <QPolygon>
#include <QObject>
#include <QColor>
#include <QList>
#include <QPointF>
#include <QDateTime>

#include <qwt_symbol.h>
#include <qwt_scale_draw.h>
#include <qwt_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_canvas.h>
#include <qwt_painter.h>
#include <qwt_plot_grid.h>
#include <qwt_text_label.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_div.h>
#include <qwt_scale_engine.h>
#include <qwt_curve_fitter.h>

#include "qwt_slrplot_picker.h"
#include "qwt_transform.h"
#include "plot.h"

class ErrorPlot : public Plot
{
    Q_OBJECT

public:
    ErrorPlot( QWidget *parent = nullptr, QString title = "");
    void setSamples( const QVector<QPointF> &samples );

    QVector<QPointF> getThreshSamples() const;


private:
    QwtPlotMarker *mark_thresh1;
    QwtPlotMarker *mark_thresh2;
};
