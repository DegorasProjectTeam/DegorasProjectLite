#include "includes/class_spaceobject.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QRegularExpression>


SpaceObjectSet::SpaceObjectSet(const QString &name, const QStringList &enabled) : name(name), list_enabled(enabled)
{

}


const QMap<SpaceObject::EnablementPolicy, QString> SpaceObject::EnablementPolicyStringMap = {
    {SpaceObject::EnablementPolicy::NO_ENABLEMENT_POLICY, "No Policy"},
    {SpaceObject::EnablementPolicy::DISABLED, "Disabled"},
    {SpaceObject::EnablementPolicy::ENABLED, "Enabled"},
    {SpaceObject::EnablementPolicy::ALWAYS_DISABLED, "Always Disabled"},
    {SpaceObject::EnablementPolicy::ALWAYS_ENABLED, "Always Enabled"}
};

const QMap<QString, SpaceObject::EnablementPolicy> SpaceObject::EnablementPolicyEnumMap = {
    {"No Policy", SpaceObject::EnablementPolicy::NO_ENABLEMENT_POLICY},
    {"Disabled", SpaceObject::EnablementPolicy::DISABLED},
    {"Enabled", SpaceObject::EnablementPolicy::ENABLED},
    {"Always Disabled", SpaceObject::EnablementPolicy::ALWAYS_DISABLED},
    {"Always Enabled", SpaceObject::EnablementPolicy::ALWAYS_ENABLED}
};

const QMap<SpaceObject::TrackPolicy, QString> SpaceObject::TrackPolicyStringMap =
{
    {SpaceObject::TrackPolicy::NO_TRACK_POLICY, "No Policy"},
    {SpaceObject::TrackPolicy::TRACK_ALWAYS, "Always"},
    {SpaceObject::TrackPolicy::TRACK_ONLY_IF_VISIBLE, "Only If Visible"},
    {SpaceObject::TrackPolicy::TRACK_ONLY_AT_NIGHT, "Only At Night"},
    {SpaceObject::TrackPolicy::CUSTOM_TRACK_POLICY_1, "Custom Track Policy 1"},
    {SpaceObject::TrackPolicy::CUSTOM_TRACK_POLICY_2, "Custom Track Policy 2"}
};

// To string method.
SpaceObject::SpaceObject(const QJsonValue &json_value, const QStringList& extrakeys) :
    name(json_value["Name"].toString()),
    ILRSname(json_value["ILRSName"].toString()),
    abbreviation(json_value["Abbreviation"].toString()),
    cospar(json_value["COSPAR"].toString()),
    ILRSID(json_value["ILRSID"].toString()),
    norad(json_value["NORAD"].toString().remove(QRegularExpression("^0*"))),
    sic(json_value["SIC"].toString()),
    classification(json_value["Classification"].toString()),
    laserid(json_value["LaserID"].toString()),
    detectorid(json_value["DetectorID"].toString()),
    counterid(json_value["CounterID"].toString()),
    cpfprovider(json_value["ProviderCPF"].toString()),
    picture(json_value["Picture"].toString()),
    trackpolicy(static_cast<TrackPolicy>(json_value["TrackPolicy"].toInt())),
    enablementpolicy(static_cast<EnablementPolicy>(json_value["EnablementPolicy"].toInt())),
    npi(json_value["NormalPointIndicator"].toInt()),
    bs(json_value["BinSize"].toInt()),
    altitude(json_value["Altitude"].toInt()),
    amplification(json_value["Amplification"].toInt()),
    priority(json_value["Priority"].toInt()),
    inclination(json_value["Inclination"].toDouble()),
    rcs(json_value["RadarCrossSection"].toDouble()),
    lrr(json_value["LaserRetroReflector"].toInt()),
    debris(json_value["IsDebris"].toInt()),
    llr_checked(true),
    debris_checked(true),
    initialized(true)
{
    QMap<QString, QString> extraparameters;

    for(const auto& key : extrakeys)
        extraparameters.insert(key, json_value[key].toString());
}

const QString SpaceObject::toString() const
{
        QString string;
        string.append("[Name:"+name+";");
        string.append("ILRSname:"+ILRSname+";");
        string.append("Abbreviation:"+abbreviation+";");
        string.append("COSPAR:"+cospar+";");
        string.append("ILRSID:"+ILRSID+";");
        string.append("NORAD:"+norad+";");
        string.append("SIC:"+sic+";");
        string.append("Classification:"+classification+";");
        string.append("LaserID:"+laserid+";");
        string.append("DetectorID:"+detectorid+";");
        string.append("CounterID:"+counterid+";");
        string.append("CPFProvider:"+cpfprovider+";");
        string.append("Picture:"+picture+";");
        string.append("TrackPolicy:"+QString::number(trackpolicy)+";");
        string.append("EnablementPolicy:"+QString::number(enablementpolicy)+";");
        string.append("NormalPointIndicator:"+QString::number(npi)+";");
        string.append("BinSize:"+QString::number(bs)+";");
        string.append("Altitude:"+QString::number(altitude)+";");
        string.append("Amplification:"+QString::number(amplification)+";");
        string.append("Priority:"+QString::number(priority)+";");
        string.append("Inclination:"+QString::number(inclination)+";");
        string.append("RadarCrossSection:"+QString::number(rcs)+";");
        string.append("LaserRetroReflector:"+QString(lrr ? "true" : "false")+";");
        string.append("Debris:"+QString(debris ? "true" : "false")+";");
        // TODO extraparameters to string QMap<QString, QString> extraparameters;
        string.append("Initialized:"+QString(initialized ? "true" : "false")+";]");
        return string;
}

// TO JSON method.
const QJsonObject SpaceObject::toJson() const
{
    QJsonObject object;
    object.insert("NORAD", QJsonValue::fromVariant(this->norad));
    object.insert("COSPAR", QJsonValue::fromVariant(this->cospar));
    object.insert("ILRSID", QJsonValue::fromVariant(this->ILRSID));
    object.insert("SIC", QJsonValue::fromVariant(this->sic));
    object.insert("Name", QJsonValue::fromVariant(this->name));
    object.insert("ILRSName", QJsonValue::fromVariant(this->ILRSname));
    object.insert("Abbreviation", QJsonValue::fromVariant(this->abbreviation));
    object.insert("Classification", QJsonValue::fromVariant(this->classification));
    object.insert("ProviderCPF", QJsonValue::fromVariant(this->cpfprovider));
    object.insert("LaserID", QJsonValue::fromVariant(this->laserid));
    object.insert("DetectorID", QJsonValue::fromVariant(this->detectorid));
    object.insert("CounterID", QJsonValue::fromVariant(this->counterid));
    object.insert("Picture", QJsonValue::fromVariant(this->picture));
    object.insert("TrackPolicy", QJsonValue::fromVariant(this->trackpolicy));
    object.insert("EnablementPolicy", QJsonValue::fromVariant(this->enablementpolicy));
    object.insert("NormalPointIndicator", QJsonValue::fromVariant(this->npi));
    object.insert("BinSize", QJsonValue::fromVariant(this->bs));
    object.insert("Altitude", QJsonValue::fromVariant(this->altitude));
    object.insert("Inclination", QJsonValue::fromVariant(this->inclination));
    object.insert("RadarCrossSection", QJsonValue::fromVariant(this->rcs));
    object.insert("LaserRetroReflector", QJsonValue::fromVariant(this->lrr));
    object.insert("Amplification", QJsonValue::fromVariant(this->amplification));
    object.insert("Priority", QJsonValue::fromVariant(this->priority));
    object.insert("IsDebris", QJsonValue::fromVariant(this->debris));
    foreach(QString i, this->extraparameters.keys())
        object.insert(i, QJsonValue::fromVariant(this->extraparameters[i]));
    return object;
}


