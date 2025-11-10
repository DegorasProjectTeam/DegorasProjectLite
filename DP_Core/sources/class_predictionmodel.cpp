#include "includes/class_predictionmodel.h"
#include "includes/class_prediction.h"
#include "includes/global_texts.h"
#include "includes/class_globalutils.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>
#include <QBrush>
#include <QIcon>


PredictionModel::PredictionModel(QObject * parent): JsonTableModel (parent)
{}

bool PredictionModel::checkMandatoryData(const QJsonValue &value, const QString &type) const
{
    if (type == "Pass" && value.isString() && !value.toString().isEmpty())
    {
        QStringList pass_splitted = value.toString().split('-');
        return pass_splitted.size() == 2 &&
                (pass_splitted[1].toLower() == "ccw" || pass_splitted[1].toLower() == "cw");
    }
    else
    {
        return JsonTableModel::checkMandatoryData(value,type);
    }
}

QVariant PredictionModel::jsonvalueToQvariant(const QJsonValue &value, const QString &type, int role) const
{
    if (type == "Pass" && !value.toString().isEmpty())
    {
        QStringList pass_splitted = value.toString().split('-');
        if (pass_splitted.size() == 2)
        {
            return pass_splitted[0];
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        return JsonTableModel::jsonvalueToQvariant(value, type, role);
    }
}

QVariant PredictionModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() < rowCount() && index.column() >= 0 && index.column() < columnCount())
    {
        QString column_index = header[index.column()]["Index"];
        if ("Azimuths" == column_index && Qt::DisplayRole == role)
        {
            QString az_start = QString::number(jsonarray[index.row()].toObject()["AzimuthStart"].toInt());
            QString az_mid = QString::number(jsonarray[index.row()].toObject()["AzimuthMiddle"].toInt());
            QString az_end = QString::number(jsonarray[index.row()].toObject()["AzimuthEnd"].toInt());

            return az_start + " " + az_mid + " " + az_end;
        }
        else if ("Duration" == column_index && Qt::DisplayRole == role)
        {
            QString start_date = jsonarray[index.row()].toObject()["DateTimeStart"].toString();
            QString end_date = jsonarray[index.row()].toObject()["DateTimeEnd"].toString();

            return QDateTime::fromString(start_date, DATETIME_STORESTRING).secsTo(
                        QDateTime::fromString(end_date, DATETIME_STORESTRING)) / 60;
        }
        else if ("MaximumElevationTime" == column_index && Qt::DisplayRole == role)
        {
            QString max_elev = QString::number(jsonarray[index.row()].toObject()["MaximumElevation"].toInt());
            QString middle_time = jsonarray[index.row()].toObject()["DateTimeMiddle"].toString();
            QString hour = QDateTime::fromString(middle_time, DATETIME_STORESTRING).toString("hh:mm");

            return max_elev + " - " + hour;
        }
        else if ("Pass" == column_index && Qt::DecorationRole == role)
        {
            QStringList pass_splitted = jsonarray[index.row()].toObject()["Pass"].toString().simplified().split('-');
            if (pass_splitted.size() == 2)
            {
                QString motion_direction = (pass_splitted[1].toLower() == "cw") ?
                    ICON_CLOCKWISE : ICON_COUNTERCLOCKWISE;
                QPixmap pix(motion_direction);
                QPixmap scaled = pix.scaled(QSize(15, 15), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QIcon icon(scaled);
                return icon;
            }
            else
            {
                return QVariant();
            }
        }
        else
        {
            return JsonTableModel::data(index,role);
        }
    }
    else
    {
        return QVariant();
    }
}


QList<Prediction*> PredictionModel::getObjectsList(const SpaceObjectsMap &map_space) const
{
    QList<Prediction*> list;
    for(const auto& object : this->jsonarray)
    {
        QString norad = object["Object_NORAD"].toString();
        auto it = map_space.find(norad);
        if(it != map_space.end())
        {
            Prediction* prediction = new Prediction(*it, object);
            if(prediction!=nullptr)
                list.append(prediction);
        }
    }
    return list;
}

Prediction* PredictionModel::getRowObject(int row, const std::shared_ptr<SpaceObject>& object) const
{
    Prediction* prediction = nullptr;
    if(row>=0 && row<this->jsonarray.size())
        prediction = new Prediction(object, this->jsonarray[row]);
    return prediction;
}
