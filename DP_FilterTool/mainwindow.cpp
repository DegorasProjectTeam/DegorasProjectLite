#include "mainwindow.h"
#include "plot.h"
#include <qmath.h>

#include <QFile>
#include <QTime>
#include <QList>
#include <QTextStream>
#include <QStringList>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>

#include "plot.h"
#include "class_cpf.h"
#include "cpfutils.h"
#include "common.h"
#include <utils.h>

#include <class_trackingfilemanager.h>
#include <class_salarasettings.h>

#include <set>

// Helper
class TrackingData
{
public:

    struct Echo
    {
        Echo(long long unsigned int t, long long int ft, long long int diff, long long int diff_cm,
             double az, double el, bool v, int mjd):
            time(t), flight_time(ft), difference(diff), difference_cm(diff_cm),
            azimuth(az), elevation(el), valid(v), mjd(mjd){}
        long long unsigned int time;  // Nanoseconds.
        long long int flight_time;    // Picosegundos.
        long long int difference;
        long long int difference_cm;
        double azimuth;
        double elevation;
        bool valid;
        int mjd;
    };


    TrackingData(QString path_file, bool reset_tracing = true)
    {
        // Lectura del fichero de seguimiento.
        QFile file(path_file);
        this->file_name = file.fileName();
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream in(&file);
            if (this->file_name.contains("dat"))
            {
                QSet<long long> selected;
                qInfo() << "ET filtered file format";
                this->et_filtered_tracking = true;

                this->satel_name = file.fileName();

                while (!in.atEnd())
                {
                    QStringList splitter = in.readLine().simplified().replace(" ", "").split(';');
                    if (splitter.size() == 2)
                    {
                        selected.insert(splitter[0].toLongLong());
                    }
                }


                QString full_filepath = QFileDialog::getOpenFileName(nullptr, "Get full ET file",  "C:/Users/ROASLR2/Documents/Auxiliar/RAW_DATA");

                QFile full_file(full_filepath);
                if (full_file.open(QIODevice::ReadOnly))
                {
                    QTextStream full(&full_file);
                    // Saltamos la primera línea
                    QString aux = full.readLine();

                    // Recorremos el resto del fichero.
                    while (!full.atEnd())
                    {
                        QStringList splitter = full.readLine().simplified().replace(" ", "").split(';');

                        // Fichero de pruebas ET: Ttiro(ns) Tvuelo(ps) Rinterp(m) Rinterp(ps)
                        if(splitter.size()==4 && selected.contains(static_cast<long long>(splitter[0].toDouble())))
                        {
                            QString time_string = splitter[0];
                            long long int flight_time = splitter[1].toDouble();

                            long long int difference = (flight_time / 2) - splitter[3].toDouble();

                            double difference_cm = difference*0.0299792458; // No se si está bien.

                            // FIXME this is a ñapa. Eliminate outliers
                            if (difference_cm < 300000)
                            {
                                long long unsigned int time = time_string.toDouble();
                                //qInfo() << "t: " << time << ", diff: " << difference;
                                this->list_echoes.append(
                                            new Echo(time, flight_time, difference, difference_cm, 0, 0, true, 0));
                            }
                        }
                    }
                }
            }
            else if (this->file_name.contains("txt"))
            {
                qInfo() << "ET file format";
                this->et_tracking = true;
                // Saltamos la primera línea
                QString aux = in.readLine();

                this->satel_name = file.fileName();

                // Recorremos el resto del fichero.
                while (!in.atEnd())
                {
                    QStringList splitter = in.readLine().simplified().replace(" ", "").split(';');

                    // Fichero de pruebas ET: Ttiro(ns) Tvuelo(ps) Rinterp(m) Rinterp(ps)
                    if(splitter.size()==4)
                    {
                        QString time_string = splitter[0];
                        long long int flight_time = splitter[1].toDouble();

                        long long int difference = (flight_time / 2) - splitter[3].toDouble();

                        double difference_cm = difference*0.0299792458; // No se si está bien.

                        // FIXME this is a ñapa. Eliminate outliers
                        if (difference_cm < 300000)
                        {
                            long long unsigned int time = time_string.toDouble();
                            //qInfo() << "t: " << time << ", diff: " << difference;
                            this->list_echoes.append(
                                        new Echo(time, flight_time, difference, difference_cm, 0, 0, true, 0));
                        }
                    }
                }
            }
            else if (this->file_name.contains("dptr"))
            {
                qInfo() << "New DP Tracking file";
                this->dp_tracking = true;
                QFileInfo info(file);

                SalaraInformation errors = TrackingFileManager::readTracking(info.fileName(), info.absolutePath(), this->data);

                if (errors.hasError())
                {
                    errors.showErrors("Filter Tool", SalaraInformation::WARNING, "");
                    return;
                }

                this->mean_cal = this->data.cal_val_overall;

                qint64 mjd = this->data.date_start.date().toJulianDay() + dpslr::utils::kJulianToModifiedJulian;
                long double prev_start = -1.L;
                long double offset = 0.L;

                for (const auto& shot : this->data.ranges)
                {
                    if (shot.start_time < prev_start)
                    {
                        offset += 86400.L;
                    }
                    prev_start = shot.start_time;

                    double resid = shot.tof_2w - shot.pre_2w - shot.trop_corr_2w - static_cast<long long>(this->data.cal_val_overall);
                    if (reset_tracing || shot.flag == Tracking::RangeData::FilterFlag::DATA )
                        this->list_echoes.append(new Echo(static_cast<unsigned long long>((shot.start_time + offset) * 1e9),
                                                          static_cast<long long>(shot.tof_2w),
                                                          static_cast<long long>(resid),
                                                          static_cast<long long>(resid*0.0299792458), {}, {}, true, mjd));
                    else if (shot.flag == Tracking::RangeData::FilterFlag::NOISE)
                        this->list_noise.append(new Echo( static_cast<unsigned long long>((shot.start_time + offset) * 1e9),
                                                          static_cast<long long>(shot.tof_2w),
                                                          static_cast<long long>(resid),
                                                          static_cast<long long>(resid*0.0299792458), {}, {}, true, mjd));
                }

                this->satel_name = this->data.obj_name;

            }

            else
            {
                // ECOS BRUTOS

                int yy = 2000 + this->file_name.split('/').last().mid(0, 2).toInt();
                int MM = this->file_name.split('/').last().mid(2, 2).toInt();
                int dd = this->file_name.split('/').last().mid(4, 2).toInt();

                int mjd = QDate(yy, MM, dd).toJulianDay() + dpslr::utils::kJulianToModifiedJulian;


                // Saltamos las 11 primeras líneas.
                for(int i=0; i<=10; i++)
                {
                    QString aux = in.readLine();
                    if(i==1)
                        this->satel_name = aux.simplified().split(',')[0];
                    else if(i==9)
                        this->mean_cal = aux.simplified().split(',')[0].toInt();
                }
                // Recorremos el resto del fichero.
                while (!in.atEnd())
                {
                    QStringList splitter = in.readLine().simplified().replace(" ", "").split(',');

                    // TODO: memory leak

                    // De momento estos son los campos del fichero antiguo del ROA. Ya se actualizará en el futuro.
                    //  Hay que tener en cuenta que a partir de aqui la función es completamente dependiente del formato
                    //  del fichero antiguo. Espero poder realizar todo esto mediante plugins en el futuro.
                    if(splitter.size()==10)
                    {
                        QString time_string = splitter[0];
                        long long int flight_time = splitter[1].toDouble();
                        long long int difference = splitter[2].toDouble();
                        double azimuth = splitter[8].toDouble();
                        double elevation = splitter[9].toDouble();
                        double difference_cm = difference*0.0299792458;

                        time_string.append("00"); // Para pasar la cifra a nanosegundos.

                        if(reset_tracing == true && time_string[0]=='-')
                        {
                            time_string.remove(0,1);
                            long long unsigned int time = time_string.toULongLong();
                            this->list_echoes.append(new Echo(time, flight_time, difference, difference_cm, azimuth, elevation, true, mjd));
                        }
                        else if(reset_tracing == false && time_string[0]=='-')
                        {
                            time_string.remove(0,1);
                            long long unsigned int time = time_string.toULongLong();
                            this->list_noise.append(new Echo(time, flight_time, difference, difference_cm, azimuth, elevation, true, mjd));
                        }
                        else
                        {
                            long long unsigned int time = time_string.toULongLong();
                            this->list_echoes.append(new Echo(time, flight_time, difference, difference_cm, azimuth, elevation, true, mjd));
                        }
                    }
                }

            }
            file.close();
            this->list_all = this->listEchoes() + this->listNoise();
            std::sort(this->list_all.begin(), this->list_all.end(),
                      [](const auto& a, const auto& b){return a->time < b->time;});
        }
    }

    const QList<Echo*>& listEchoes() const {return this->list_echoes;}
    const QList<Echo*>& listNoise() const {return this->list_noise;}
    const QList<Echo*>& listAll() const {return this->list_all;}
    int meanCal() const {return this->mean_cal;}
    QString satel_name;
    QString file_name;
    // Temporary variable
    bool et_tracking = false;
    bool et_filtered_tracking = false;
    bool dp_tracking = false;
    Tracking data;

private:
    QList<Echo*> list_echoes;
    QList<Echo*> list_noise;
    QList<Echo*> list_all;
    int mean_cal;
    // Falta meter la clase satélite como un miembro más. Pendiente de reestructuración.
};

MainWindow::MainWindow()
{
    this->changed = false;
    QPolygonF samples, selected;
    QString path_test = SalaraSettings::instance().getGlobalConfigString("SalaraProjectDataPaths/SP_CurrentObservations");

    QWidget * wdg = new QWidget(this);

    QVBoxLayout *vlay = new QVBoxLayout(wdg);
    QHBoxLayout *hlay = new QHBoxLayout;

    QPushButton *btnDiscard = new QPushButton("Descartar");
    btnDiscard->setAccessibleName("ButtonRed");
    btnDiscard->setIcon(QIcon(":/icons/round_delete_icon&48.png"));
    btnDiscard->setMinimumHeight(25);
    QPushButton *btnSave = new QPushButton("Guardar");
    btnSave->setAccessibleName("ButtonGreen");
    btnSave->setIcon(QIcon(":/icons/save_icon&48.png"));
    btnSave->setMinimumHeight(25);
    QPushButton *btnCalc = new QPushButton("Calc. Estadísticas.");
    btnCalc->setAccessibleName("ButtonLilac");
    btnCalc->setIcon(QIcon(":/icons/cogs_icon&48.png"));
    btnCalc->setMinimumHeight(25);
    QPushButton *btnRecalcResids = new QPushButton("Volver a calcular residuos");
    QPushButton *btnThreshFilter = new QPushButton("Filtrar por RMS");
    QPushButton *btnAutoFilter = new QPushButton("Autofiltrado");
    btnAutoFilter->setAccessibleName("ButtonLilac");
    btnAutoFilter->setIcon(QIcon(":/icons/brush_icon&48.png"));
    btnAutoFilter->setMinimumHeight(25);

    btnAutoFilter->setEnabled(false);
    btnSave->setEnabled(false);
    btnCalc->setEnabled(false);
    btnThreshFilter->setEnabled(false);

    d_plot = new Plot(wdg);
    d_plot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    error_plot = new ErrorPlot(wdg);
    error_plot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    hlay->addWidget(btnThreshFilter);
    hlay->addWidget(btnAutoFilter);
    hlay->addWidget(btnCalc);
    hlay->addWidget(btnSave);
    hlay->addWidget(btnRecalcResids);
    hlay->addWidget(btnDiscard);
    vlay->addLayout(hlay);
    vlay->addWidget(d_plot);
    vlay->addWidget(error_plot);
    vlay->setStretch(0, 1);
    vlay->setStretch(1, 4);
    vlay->setStretch(2, 4);

    setCentralWidget(wdg);


    QObject::connect(d_plot, &Plot::selectionChanged, [this]
    {
        auto fit_errors = this->d_plot->getFitErrors();
        this->error_plot->setSamples(fit_errors);
    });

    QObject::connect(this->error_plot, &ErrorPlot::selectionChanged, [this]
    {
        auto error_samples = this->error_plot->getSelectedSamples();
        auto samples = this->d_plot->getSelectedSamples();
        decltype(samples) selected_samples;
        std::copy_if(std::make_move_iterator(samples.begin()), std::make_move_iterator(samples.end()),
                     std::back_inserter(selected_samples), [&error_samples](const auto& p)
        {
            auto it = std::find_if(error_samples.begin(), error_samples.end(),
                                   [&p](const auto& t){return p.x() == t.x();});
            return it != error_samples.end();
        });
        this->d_plot->setSamples(selected_samples);
    });

    QObject::connect(this->error_plot, &ErrorPlot::startedPicking, this, [this]{this->d_plot->setPickingEnabled(false);});
    QObject::connect(this->error_plot, &ErrorPlot::finishedPicking, this, [this]
    {
        this->d_plot->setPickingEnabled(true);
        emit this->filterChanged();
    });
    QObject::connect(this->d_plot, &ErrorPlot::startedPicking, this, [this]{this->error_plot->setPickingEnabled(false);});
    QObject::connect(this->d_plot, &ErrorPlot::finishedPicking, this, [this]
    {
        this->error_plot->setPickingEnabled(true);
        emit this->filterChanged();
    });

    QObject::connect(this, &MainWindow::filterChanged, this, [this, btnCalc, btnSave, btnThreshFilter, btnAutoFilter]
    {
        btnCalc->setEnabled(true);
        btnSave->setEnabled(true);
        btnThreshFilter->setEnabled(true);
        btnAutoFilter->setEnabled(true);
        this->changed = true;
    });

    QObject::connect(this, &MainWindow::filterSaved, this, [this, btnCalc, btnSave, btnThreshFilter, btnAutoFilter]
    {
        btnCalc->setEnabled(false);
        btnSave->setEnabled(false);
        btnThreshFilter->setEnabled(false);
        btnAutoFilter->setEnabled(false);
        this->changed = false;
    });

    // TODO: enable buttons

    QStringList arguments = QApplication::instance()->arguments();
    QString file;
    if (arguments.size() > 1)
        file = arguments[1];
    else
        file = QFileDialog::getOpenFileName(this, "", path_test);

    if (file.isEmpty())
    {
        QApplication::exit();
        std::exit(0);
    }

    this->tracking = new TrackingData(file, false);

    this->file_name = this->tracking->file_name;

    for (const auto* aux : this->tracking->listAll())
    {
        samples += QPointF(aux->time, aux->difference);
    }

    for (const auto* aux : this->tracking->listEchoes())
    {
        selected += QPointF(aux->time, aux->difference);
    }

    this->d_plot->setTitle("Tracking: "+ this->tracking->satel_name);
    this->d_plot->setSamples(samples);
    this->d_plot->setBinSize(this->tracking->data.obj_bs);
    this->error_plot->setBinSize(this->tracking->data.obj_bs);

    if(samples.size()!=selected.size())
        d_plot->selected_curve->setSamples(selected);

    btnDiscard->setEnabled(this->tracking->dp_tracking);

    QObject::connect(btnAutoFilter, &QPushButton::clicked, this, [this, hlay]
    {
        hlay->setEnabled(false);
        QProgressDialog pd("Autofiltrado en proceso.", "", 0, 0, this);
        pd.setCancelButton(nullptr);

        auto future = QtConcurrent::run([this]
        {
            int i = 0;
            int changed;
            do
            {
                QMetaObject::invokeMethod(this, &MainWindow::threshFilter, Qt::BlockingQueuedConnection, &changed);
                i++;
            } while(changed > 0 && i < 20);
        });

        QFutureWatcher<void> fw;
        QObject::connect(&fw, &QFutureWatcher<void>::finished, &pd, &QProgressDialog::accept, Qt::QueuedConnection);
        fw.setFuture(future);
        pd.exec();

        hlay->setEnabled(true);
    });

    QObject::connect(btnThreshFilter, &QPushButton::clicked, this, &MainWindow::threshFilter);

    QObject::connect(btnDiscard, &QPushButton::clicked, [this]
    {
        if (!this->tracking->dp_tracking)
            return;

        QMessageBox::StandardButton result =
                QMessageBox::question(this, "Filter Tool",
                                      "¿Descartar este seguimiento? Se guardará como seguido sin resultados.",
                                      {QMessageBox::Yes | QMessageBox::No}, QMessageBox::No);

        if (QMessageBox::No == result)
            return;


        for (auto&& shot : this->tracking->data.ranges)
        {
            shot.flag = Tracking::RangeData::FilterFlag::NOISE;
        }

        dpslr::algorithms::DistStats zero_stats{0,0,0,0.L,0.L,0.L,0.L,0.L,0.};

        this->tracking->data.stats_1rms = zero_stats;
        this->tracking->data.stats_rfrms = zero_stats;

        this->tracking->data.filter_mode = Tracking::FilterMode::MANUAL;
        SalaraInformation errors = TrackingFileManager::writeTracking(this->tracking->data);

        if (errors.hasError())
            errors.showErrors("Filter Tool", SalaraInformation::WARNING, "Error al guardar fichero descartado.", this);
        else
        {
            QString current_path =
                    SalaraSettings::instance().getGlobalConfigString("SalaraProjectDataPaths/SP_CurrentObservations");
            QString filename = TrackingFileManager::trackingFilename(this->tracking->data);
            bool result = QFile::remove(current_path + '/' + filename);
            if (result)
            {
                SalaraInformation::showInfo("Filter Tool", "Fichero descartado correctamente. Puede cerrar esta ventana",
                                            "", this);
                this->changed = false;
                emit this->filterSaved();
            }
            else
                SalaraInformation::showInfo("Filter Tool", "No se pudo eliminar el seguimiento. Descártelo manualmente.",
                                            "", this);
        }

    });

    QObject::connect(btnSave, &QPushButton::clicked, [this]
    {
        QVector<QPointF> v = this->d_plot->getSelectedSamples();


        if (this->tracking->dp_tracking)
        {
            QwtSLRArraySeriesData *selected_data =
                    static_cast<QwtSLRArraySeriesData *>(this->d_plot->selected_curve->data());
            QVector<QPointF> v = selected_data->samples();

            if (0 == v.size())
            {
                SalaraInformation::showWarning("Filter Tool",
                                               "No hay puntos seleccionados. ¿Ha calculado las estadísticas?", "", this);
            }
            else
            {

                qInfo() << QString("Saving as data %1 points").arg(v.size());

                std::set<qulonglong> selected;

                std::transform(v.begin(), v.end(), std::inserter(selected, selected.begin()),
                               [](const auto& e){return static_cast<qulonglong>(e.x());});

                long double prev_start = -1.L;
                long double offset = 0.L;

                for (auto&& shot : this->tracking->data.ranges)
                {
                    if (shot.start_time < prev_start)
                    {
                        offset += 86400.L;
                    }
                    prev_start = shot.start_time;
                    // TODO: Unknown shots
                    if (selected.find(static_cast<qulonglong>((shot.start_time + offset) * 1e9L)) != selected.cend())
                        shot.flag = Tracking::RangeData::FilterFlag::DATA;
                    else
                        shot.flag = Tracking::RangeData::FilterFlag::NOISE;
                }

                this->tracking->data.filter_mode = Tracking::FilterMode::MANUAL;
                SalaraInformation errors = TrackingFileManager::writeTracking(this->tracking->data);

                if (errors.hasError())
                    errors.showErrors("Filter Tool", SalaraInformation::WARNING, "Error on file saving", this);
                else
                {
                    SalaraInformation::showInfo("Filter Tool",
                                                "Data saved successfully to current and historical observations path",
                                                "", this);
                    emit this->filterSaved();
                }
            }
        }

        else
        {
            QFile file(this->file_name+"_filtered.dat");
            file.open(QIODevice::ReadWrite | QIODevice::Text);
            QTextStream stream( &file );
            stream.setRealNumberNotation(QTextStream::FixedNotation);
            int i=0;
            for(const auto& p : std::as_const(v))
            {
                auto x = static_cast<qulonglong>(p.x());
                if (this->tracking->et_tracking)
                {
                    auto echo = std::find_if(this->tracking->listEchoes().begin(), this->tracking->listEchoes().end(),
                                             [x](TrackingData::Echo* e) {return x == e->time;});
                    if (echo != this->tracking->listEchoes().end())
                        stream<<x<<";"<<static_cast<qlonglong>((*echo)->flight_time / 2);
                    else
                        qInfo() << "Unknown point with time: " << x;
                }
                else
                {

                    stream<<x<<";"<<static_cast<qlonglong>(p.y());


                }

                i++;
                if(i<v.size())
                    stream<<"\n";
            }
            file.close();

            if (!this->tracking->et_tracking)
            {
                QFile original(this->file_name);
                QFileInfo original_info(this->file_name);

                QString destino = "C:/LASER/DATOS/CorrecciónEcosBrutos/Filtrados";

                QFile dest(destino + '/' + original_info.baseName() + "_filt." + original_info.suffix());
                if (original.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QTextStream in(&original);
                    QTextStream out(&dest);

                    dest.open(QIODevice::WriteOnly);

                    // Copy header
                    for (int i = 0; i < 10; i++)
                    {
                        out << in.readLine() << Qt::endl;
                    }

                    while(!in.atEnd())
                    {
                        QString line  = in.readLine();
                        QStringList splitter = line.split(',');

                        if (!splitter.empty())
                        {
                            long long time = splitter[0].toLongLong() * 100L;
                            auto it = std::find_if(v.begin(), v.end(),
                                                   [t = std::abs(time)](const auto& p){return t == p.x();});
                            // This echo is selected
                            if (it != v.end())
                            {
                                line[0] = ' ';
                            }
                            // This echo is not selected
                            else
                            {
                                line[0] = '-';
                            }
                        }

                        out << line << Qt::endl;

                    }
                    dest.close();
                }
                original.close();
            }

            emit this->filterSaved();
        }


    });

    QObject::connect(btnCalc, &QPushButton::clicked, [this]
    {

        if (this->tracking->dp_tracking)
        {
            QVector<QPointF> v = this->d_plot->getSelectedSamples();
            QVector<QPointF> fitted_samples = this->error_plot->getSelectedSamples();
            QVector<QPointF> error;
            QVector<QPointF> fitted_error;

            std::set<qulonglong> selected;
            dpslr::common::RangeData rd;
            dpslr::common::ResidualsData<> resid;

            std::transform(v.begin(), v.end(), std::inserter(selected, selected.begin()),
                           [](const auto& e){return static_cast<qulonglong>(e.x());});

            long double prev_start = -1.L;
            long double offset = 0.L;


            for (auto&& shot : this->tracking->data.ranges)
            {
                if (shot.start_time < prev_start)
                {
                    offset += 86400.L;
                }
                prev_start = shot.start_time;

                if (selected.find(static_cast<qulonglong>((shot.start_time + offset) * 1e9)) != selected.cend())
                    rd.emplace_back(shot.start_time, shot.tof_2w - static_cast<long long>(this->tracking->data.cal_val_overall),
                                    shot.pre_2w, shot.trop_corr_2w);
            }

            auto res_error = dpslr::algorithms::calculateFullRateResiduals(rd, this->tracking->data.obj_bs, resid);

            if (res_error != dpslr::algorithms::FullRateResCalcErr::NOT_ERROR)
            {
                SalaraInformation::showWarning("Filter Tool", "Residuals calculation failed. Error code: " +
                                               QString::number(static_cast<int>(res_error)), "", this);
                return;
            }

            dpslr::algorithms::ResidualsStats res_stats;
            auto calc_error = dpslr::algorithms::calculateResidualsStats(this->tracking->data.obj_bs, resid, res_stats);

            if (calc_error != dpslr::algorithms::ResiStatsCalcErr::NOT_ERROR)
            {
                SalaraInformation::showWarning("Filter Tool", "Statistics calculation failed. Error code: " +
                                               QString::number(static_cast<int>(calc_error)), "", this);
                return;
            }

            for (int idx = res_stats.total_bin_stats.amask_rfrms.size() - 1; idx >= 0; idx--)
                if (!res_stats.total_bin_stats.amask_rfrms[idx])
                {
                    error.push_back(v.takeAt(idx));
                    fitted_error.push_back(fitted_samples.takeAt(idx));
                }

            this->d_plot->selected_curve->setSamples(v);
            this->d_plot->error_curve->setSamples(error);
            this->error_plot->selected_curve->setSamples(fitted_samples);
            this->error_plot->error_curve->setSamples(fitted_error);


            this->d_plot->replot();
            this->error_plot->replot();

            this->tracking->data.tror_rfrms = (res_stats.total_bin_stats.stats_rfrms.aptn * 100.) / this->tracking->data.nshots;
            this->tracking->data.tror_1rms = (res_stats.total_bin_stats.stats_01rms.aptn * 100.) / this->tracking->data.nshots;
            this->tracking->data.rf = 2.5;
            this->tracking->data.stats_rfrms = res_stats.total_bin_stats.stats_rfrms;
            this->tracking->data.stats_1rms = res_stats.total_bin_stats.stats_01rms;

            QString stats = "Stats calculation result: \n Bin size: %1, RF: %2 \n Iterations: %3\n "
                            "Total Points: %4\n Accepted points: %5\n Rejected Points: %6\n "
                            "Mean: %7 \n RMS: %8\n Skew: %9\n "
                            "Kurt: %10\n Peak: %11\n Peak - mean: %12\n Accepted Rate: %13";
            QString stats_summary = stats.arg(res_stats.bs).arg(res_stats.rf).arg(res_stats.total_bin_stats.stats_rfrms.iter)
                    .arg(res_stats.total_bin_stats.ptn).arg(res_stats.total_bin_stats.stats_rfrms.aptn).arg(res_stats.total_bin_stats.stats_rfrms.rptn)
                    .arg(static_cast<double>(res_stats.total_bin_stats.stats_rfrms.mean))
                    .arg(static_cast<double>(res_stats.total_bin_stats.stats_rfrms.rms))
                    .arg(static_cast<double>(res_stats.total_bin_stats.stats_rfrms.skew))
                    .arg(static_cast<double>(res_stats.total_bin_stats.stats_rfrms.kurt))
                    .arg(static_cast<double>(res_stats.total_bin_stats.stats_rfrms.peak))
                    .arg(static_cast<double>(res_stats.total_bin_stats.stats_rfrms.peak - res_stats.total_bin_stats.stats_rfrms.mean))
                    .arg(res_stats.total_bin_stats.stats_rfrms.arate);

            SalaraInformation::showInfo("Filter Tool",
                                        "Cálculo de estadísticas realizado con éxito. Clicar Show Details para ver resumen",
                                        stats_summary, this);
        }
    });


    QObject::connect(btnRecalcResids, &QPushButton::clicked, [this, satel = this->tracking->satel_name,
                     data = this->tracking->listAll(), cal = this->tracking->meanCal()]
    {
        QVector<QPointF> samples_interp;
        dpslr::cpfutils::CPFInterpolator::InterpolationResult interp_data;
        CPF cpf;
        std::array<long double, 3> xyz = {5105473.885L, -555110.526L, 3769892.958L};
        long double lat = 36.46525556L;
        long double lon = 353.79469440L;

        //QString path = "C:/0-SALARA_PROJECT/SP_DataFiles/SP_CPF/SP_CurrentCPF";
        QString path = "C:/LASER/EfemeridesN/TODOS_CPF";
        QString file = QFileDialog::getOpenFileName(this,
            tr("Open CPF"), path, "CPF Files ("+satel+"*cpf*)");

        if(file.isEmpty())
            return;

        // Open the CPF
        if(cpf.openCPFFile(file.toStdString(), CPF::OpenOptionEnum::ALL_DATA) != CPF::ReadFileErrorEnum::NOT_ERROR)
            return;

        // Init the interpolator
        dpslr::cpfutils::CPFInterpolator interpolator(
                    cpf, {lat, lon, 98.177L, dpslr::geo::meas::Angle<long double>::Unit::DEGREES},
                    {xyz[0], xyz[1], xyz[2]});

        long long unsigned int prev_start = 0;
        unsigned int offset = 0;


        // Do the interpolations.
        for (const auto* aux : data)
        {
            if (aux->time < prev_start)
            {
                offset++;
            }
            prev_start = aux->time;

            auto error = interpolator.interpolate(aux->mjd + offset, (aux->time * 1e-9) - offset*86400, interp_data);

            if(error != dpslr::cpfutils::CPFInterpolator::InterpolationError::NOT_ERROR)
            {
                QMessageBox::warning(this, "Interpolation error", "Error: " + QString::number(error));
                return;
            }

//            qInfo() << "MJD: " << aux->mjd;
//            qInfo() << "Seconds: " <<QString::number(aux->time * 1e-9, 'g', 12);
//            qInfo() << "Flight: " << aux->flight_time;
//            qInfo() << "FInter: " << QString::number(interp_data.flight_time_2w * 1e12, 'g', 11);
//            qInfo() << "Diff: " << aux->flight_time - interp_data.flight_time_2w * 1e12;
//            qInfo() << Qt::endl;
            
            samples_interp += QPointF(aux->time, aux->flight_time - interp_data.tof_2w * 1e12 - cal);
        }

        // If all ok, then delete the current points.
//        this->d_plot->setSamples({});
//        this->error_plot->setSamples({});
//        this->d_plot->replot();
//        this->error_plot->replot();
        //this->update();

        this->d_plot->setSamples(samples_interp);

        this->update();

        //this->error_plot->setSamples(this->d_plot->getFitErrors());
    });

    QObject::connect(d_plot->getPanner(), &QwtPlotPanner::panned, [this]()
    {
        // Ajustamos Axis y repintamos. En un futuro lo modificaré para que el movimiento sea a la vez.
        QwtInterval interval = this->d_plot->axisInterval(QwtPlot::Axis::xBottom);
        this->error_plot->setAxisScale(QwtPlot::Axis::xBottom,interval.minValue(), interval.maxValue());
        this->error_plot->replot();
    });

    QObject::connect(error_plot->getPanner(), &QwtPlotPanner::panned, [this]()
    {
        // Ajustamos Axis y repintamos. En un futuro lo modificaré para que el movimiento sea a la vez.
        QwtInterval interval = this->error_plot->axisInterval(QwtPlot::Axis::xBottom);
        this->d_plot->setAxisScale(QwtPlot::Axis::xBottom,interval.minValue(), interval.maxValue());
        this->d_plot->replot();
    });
}

int MainWindow::threshFilter()
{
    auto thresh_samples = this->error_plot->getThreshSamples();
    auto samples = this->d_plot->getSelectedSamples();

    if (thresh_samples.size() != samples.size())
    {
        decltype(samples) selected_samples;
        std::copy_if(std::make_move_iterator(samples.begin()), std::make_move_iterator(samples.end()),
                     std::back_inserter(selected_samples), [&thresh_samples](const auto& p)
        {
            auto it = std::find_if(thresh_samples.begin(), thresh_samples.end(),
                                   [&p](const auto& t){return p.x() == t.x();});
            return it != thresh_samples.end();
        });
        this->d_plot->setSamples(selected_samples);
    }

    return samples.size() - thresh_samples.size();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (this->changed)
    {
        QMessageBox::StandardButton resBtn =
                QMessageBox::question( this, "Filter Tool", "Cambios en el filtrado sin guardar. ¿Cerrar sin guardar?",
                                       QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
        if (resBtn != QMessageBox::Yes)
            event->ignore();
        else
            event->accept();
    }
}
