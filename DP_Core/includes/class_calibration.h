#pragma once

#include "spcore_global.h"
#include "class_meteodata.h"

#include <class_crd.h>
#include <geo.h>
#include <algorithms.h>

#include <QString>
#include <QDateTime>


struct SP_CORE_EXPORT Calibration
{

    enum class Type
    {
        UNDEFINED,
        NOMINAL,
        EXTERNAL,
        INTERNAL_TELESCOPE,
        INTERNAL_BUILDING,
        BURST_CALIBRATIONS,
        OTHER
    };

    struct SP_CORE_EXPORT RangeData
    {
        enum class FilterFlag
        {
            UNKNOWN = 0,
            NOISE = 1,
            DATA = 2
        };

        long double start_time;
        long double tof_2w;
        FilterFlag flag;

        explicit RangeData();

    };

    QDateTime date_start;
    QString cfg_id;
    QString station_name;
    unsigned int station_id;
    Type type;
    dpslr::geo::meas::Distance<double> target_dist_2w;
    dpslr::geo::meas::Distance<double> target_tof_2w;
    double rf;
    std::size_t nshots;
    std::size_t rnshots;
    std::size_t unshots;
    double tror_rfrms;
    double tror_1rms;
    double cal_val_rfrms;
    double cal_val_1rms;

    MeteoData meteo;

    dpslr::algorithms::DistStats stats_1rms;
    dpslr::algorithms::DistStats stats_rfrms;

    std::vector<RangeData> ranges;
    std::vector<long double> tA;
    std::vector<long double> tB;
    unsigned int et_precision;

    explicit Calibration();

};

