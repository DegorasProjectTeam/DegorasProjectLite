#include "includes/class_statsfilemanager.h"

#include <QString>

const QString kIter = QStringLiteral("iter");
const QString kAPtn = QStringLiteral("andata");
const QString kRPtn = QStringLiteral("rndata");
const QString kMean = QStringLiteral("mean");
const QString kRMS = QStringLiteral("rms");
const QString kSkew = QStringLiteral("skew");
const QString kKurt = QStringLiteral("kurt");
const QString kPeak = QStringLiteral("peak");
const QString kARate = QStringLiteral("ror");

dpslr::algorithms::DistStats StatsFileManager::fromJson(const QJsonObject& o)
{
    dpslr::algorithms::DistStats stats;
    stats.iter = o[kIter].toInt();
    stats.aptn = o[kAPtn].toInt();
    stats.rptn = o[kRPtn].toInt();
    stats.mean = o[kMean].toDouble();
    stats.rms = o[kRMS].toDouble();
    stats.skew = o[kSkew].toDouble();
    stats.kurt = o[kKurt].toDouble();
    stats.peak = o[kPeak].toDouble();
    stats.arate = o[kARate].toDouble();

    return stats;
}

QJsonObject StatsFileManager::toJson(const dpslr::algorithms::DistStats &stats)
{
    QJsonObject o;
    o.insert(kIter, static_cast<int>(stats.iter));
    o.insert(kAPtn, static_cast<int>(stats.aptn));
    o.insert(kRPtn, static_cast<int>(stats.rptn));
    o.insert(kMean, static_cast<double>(stats.mean));
    o.insert(kRMS, static_cast<double>(stats.rms));
    o.insert(kSkew, static_cast<double>(stats.skew));
    o.insert(kKurt, static_cast<double>(stats.kurt));
    o.insert(kPeak, static_cast<double>(stats.peak));
    o.insert(kARate, static_cast<double>(stats.arate));
    return o;
}
