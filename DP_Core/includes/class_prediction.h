#pragma once

#include <QDateTime>

#include <memory>

#include "class_spaceobject.h"
#include "global_texts.h"
#include "spcore_global.h"


class SP_CORE_EXPORT Prediction
{
public:

    Prediction(const std::shared_ptr<SpaceObject>& object, const QJsonValue &json_value):object(object)
    {
        QString DateTimeStart = json_value["DateTimeStart"].toString();
        QString DateTimeMiddle = json_value["DateTimeMiddle"].toString();
        QString DateTimeEnd = json_value["DateTimeEnd"].toString();
        this->elevation_max = json_value["MaximumElevation"].toInt();
        this->azi_start = json_value["AzimuthStart"].toInt();
        this->azi_middle = json_value["AzimuthMiddle"].toInt();
        this->azi_end = json_value["AzimuthEnd"].toInt();
        this->cpf_source = json_value["CPFSource"].toString();
        this->pass = json_value["Pass"].toString();
        this->campaign_track = json_value["CampaignTrack"].toBool();
        this->time_start = QDateTime::fromString(DateTimeStart, DATETIME_STORESTRING);
        this->time_middle = QDateTime::fromString(DateTimeMiddle, DATETIME_STORESTRING);
        this->time_end = QDateTime::fromString(DateTimeEnd, DATETIME_STORESTRING);
        this->duration_mins = int(this->time_start.secsTo(this->time_end)/60);
    }

    Prediction(const std::shared_ptr<SpaceObject>& object, int elevation_max, int azi_start, int azi_middle, int azi_end,
                        const QDateTime& time_start, const QDateTime& time_middle, const QDateTime& time_end,
                        const QString& cpf_source, bool campaign_track = false):
                        object(object), elevation_max(elevation_max), azi_start(azi_start), azi_middle(azi_middle),
                        azi_end(azi_end), time_start(time_start), time_middle(time_middle), time_end(time_end),
                        cpf_source(cpf_source), campaign_track(campaign_track)
    {
        this->duration_mins = int(this->time_start.secsTo(this->time_end)/60);
        this->pass = this->calculateTrajectory(this->azi_start, this->azi_middle, this->azi_end);
    }

    // Inline observers.
    inline const std::shared_ptr<SpaceObject>& getSpaceObject() const {return this->object;}
    inline int getMaxElevation() const {return this->elevation_max;}
    inline int getStartAzimuth() const {return this->azi_start;}
    inline int getMiddleAzimuth() const {return this->azi_middle;}
    inline int getEndAzimuth() const {return this->azi_end;}
    inline int getDurationMinutes() const {return this->duration_mins;}
    inline const QDateTime& getStartTimeUTC() const {return this->time_start;}
    inline const QDateTime& getMiddleTimeUTC() const {return this->time_middle;}
    inline const QDateTime& getEndTimeUTC() const {return this->time_end;}
    inline const QString& getCPFSource() const {return this->cpf_source;}
    inline const bool& isCampaignTrack() const {return this->campaign_track;}
    inline const QString& getPass() const {return this->pass;}
    QString getAzimuths() const;

    // To JSON method.
    const QJsonObject toJson() const;

    // Calculate trajectory method.
    static QString calculateTrajectory(int az1, int az2, int az3);


private:

    std::shared_ptr<SpaceObject> object;
    int elevation_max;
    int azi_start;
    int azi_middle;
    int azi_end;
    int duration_mins;
    QDateTime time_start;
    QDateTime time_middle;
    QDateTime time_end;
    QString cpf_source;
    QString pass; // NE/SE-CCW
    bool campaign_track;
};

// Operator
SP_CORE_EXPORT bool operator < (const Prediction& a, const Prediction& b);


Q_DECLARE_METATYPE(Prediction*)
