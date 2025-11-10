#include "includes/class_tracking.h"
#include "includes/class_cpffilemanager.h"
#include "includes/class_globalutils.h"

Tracking::RangeData::RangeData():
    start_time(0.0),
    tof_2w(0.0),
    pre_2w(0.0),
    trop_corr_2w(0.0),
    bias(0.0),
    flag(Tracking::RangeData::FilterFlag::UNKNOWN)
{

}

Tracking::Tracking() :
    filter_mode(FilterMode::RAW),
    obj_bs(0),
    nshots(0),
    rnshots(0),
    unshots(0),
    tror_rfrms(0.0),
    tror_1rms(0.0),
    rf(0.),
    release(0),
    cal_val_overall(0.0),
    et_precision(0)

{

}

QJsonObject TelescopeData::toJson() const
{
    // TODO
    return {};
}

TelescopeData TelescopeData::fromJson()
{
    //TODO
    return TelescopeData();
}

TelescopeData::TelescopeData() :
    time(0.0),
    az(0.0),
    el(0.0),
    az_ofst(0.0),
    el_ofst(0.0),
    az_rate(0.0),
    el_rate(0.0),
    dir(TelescopeData::DirectionFlag::TRANSMIT_AND_RECEIVE),
    origin(TelescopeData::Origin::UNKNOWN)
{

}
