#pragma once

#include <algorithms.h>

#include <QJsonObject>

class StatsFileManager
{
public:

    static dpslr::algorithms::DistStats fromJson(const QJsonObject &o);
    static QJsonObject toJson(const dpslr::algorithms::DistStats &stats);
};

