#include "includes/class_prediction.h"
#include "includes/class_globalutils.h"

#include <QJsonValue>
#include <QLabel>


QString Prediction::getAzimuths() const
{
    QString line;
    line.append(QString::number(this->azi_start).rightJustified(3, ' '));
    line.append(' ');
    line.append(QString::number(this->azi_middle).rightJustified(3, ' '));
    line.append(' ');
    line.append(QString::number(this->azi_end).rightJustified(3, ' '));
    return line;
}

const QJsonObject Prediction::toJson() const
{
    QJsonObject object;

    // En pantalla.
    object.insert("Object_NORAD", QJsonValue::fromVariant(this->object->getNorad()));
    object.insert("Object_Name", QJsonValue::fromVariant(this->object->getName()));
    object.insert("DateTimeStart", QJsonValue::fromVariant(this->time_start.toString("yyyyMMdd.hhmmss")));
    object.insert("DateTimeMiddle", QJsonValue::fromVariant(this->time_middle.toString("yyyyMMdd.hhmmss")));
    object.insert("DateTimeEnd", QJsonValue::fromVariant(this->time_end.toString("yyyyMMdd.hhmmss")));
    object.insert("MaximumElevation", QJsonValue::fromVariant(this->elevation_max));
    object.insert("AzimuthStart", QJsonValue::fromVariant(this->azi_start));
    object.insert("AzimuthMiddle", QJsonValue::fromVariant(this->azi_middle));
    object.insert("AzimuthEnd", QJsonValue::fromVariant(this->azi_end));
    object.insert("CPFSource", QJsonValue::fromVariant(this->cpf_source));
    object.insert("Pass", QJsonValue::fromVariant(this->pass));
    object.insert("CampaignTrack", QJsonValue::fromVariant(this->campaign_track));
    return object;
}

QString Prediction::calculateTrajectory(int azi_1, int azi_2, int azi_3)
{
    // Variables.
    QString track, rotation;

    // First azimuth.
    if(azi_1==0 || azi_1==360)    track = "N/";
    else if(azi_1==90)            track = "E/";
    else if(azi_1==180)           track = "S/";
    else if(azi_1==270)           track = "O/";
    else if(azi_1>0&&azi_1<90)    track = "NE/";
    else if(azi_1>90&&azi_1<180)  track = "SE/";
    else if(azi_1>180&&azi_1<270) track = "SO/";
    else if(azi_1>270&&azi_1<360) track = "NO/";

    // Final azimuth.
    if(azi_3==0 || azi_3==360)    track = track+"N";
    else if(azi_3==90)            track = track+"E";
    else if(azi_3==180)           track = track+"S";
    else if(azi_3==270)           track = track+"O";
    else if(azi_3>0&&azi_3<90)    track = track+"NE";
    else if(azi_3>90&&azi_3<180)  track = track+"SE";
    else if(azi_3>180&&azi_3<270) track = track+"SO";
    else if(azi_3>270&&azi_3<360) track = track+"NO";

    // Direction of rotation;
    if(azi_1<azi_2 && azi_2<azi_3)      rotation = "CW";
    else if(azi_1>azi_2 && azi_2>azi_3) rotation = "CCW";
    else if(azi_1>azi_2 && azi_2<azi_3) rotation = "CW";
    else if(azi_1<azi_2 && azi_2>azi_3) rotation = "CW";
    else if(azi_1<azi_2 && azi_2>azi_3) rotation = "CCW";
    else if(azi_1>azi_2 && azi_2<azi_3) rotation = "CCW";
    // If azi_1 == azi_2 or azi_2 == azi_3, then search the shortest.
    else if(azi_1==azi_2 || azi_2==azi_3)
    {
        if(azi_1<azi_3)
        {
            if(azi_3-azi_1<azi_1+360-azi_3) rotation = "CW";
            else rotation ="CCW";
        }
        else if(azi_1>azi_3)
        {
            if(azi_1-azi_3<360-azi_1+azi_3) rotation = "CCW";
            else rotation ="CW";
        }
        // TODO: this should be checked
        // If the three angles are the same, then choose CW by default
        else
            rotation = "CW";
    }

    // Return the trajectory.
    return track+'-'+rotation;
}

bool operator <(const Prediction &a, const Prediction &b)
{
    return a.getStartTimeUTC()<b.getStartTimeUTC();
}
