#pragma once

#include "interface_plugin.h"
#include "class_salarainformation.h"

#include <class_tle.h>

class TLEPropagator : public SPPlugin
{
public:

    TLEPropagator():SPPlugin(PluginCategory::TLE_PROPAGATOR){}
    virtual ~TLEPropagator() override = default;

    virtual SalaraInformation propagateTLEs(const std::vector<TLE> &tles, const QString& dest_path) = 0;
};

QT_BEGIN_NAMESPACE
#define TLEPropagator_iid "DEGORAS_PROJECT.Interface_TLEPropagator"
Q_DECLARE_INTERFACE(TLEPropagator, TLEPropagator_iid)
QT_END_NAMESPACE

Q_DECLARE_METATYPE(TLEPropagator*)
