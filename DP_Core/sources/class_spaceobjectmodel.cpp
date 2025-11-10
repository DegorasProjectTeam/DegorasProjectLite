#include "includes/class_spaceobjectmodel.h"
#include "includes/class_spaceobject.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>
#include <QBrush>

SpaceObjectModel::SpaceObjectModel(QObject * parent, bool user_checkable):
    JsonTableModel (parent),
    user_checkable(user_checkable)
{}


void SpaceObjectModel::disableAll()
{
    for(int i =0; i<this->jsonarray.count();i++)
    {
        QJsonObject obj = jsonarray[i].toObject();
        SpaceObject::EnablementPolicy policy = static_cast<SpaceObject::EnablementPolicy>(
                    obj["EnablementPolicy"].toInt());
        int en_policy_column = this->findColumnSectionByIndex("EnablementPolicy");
        // Only enabled or no policy members will change to disabled
        if (policy == SpaceObject::EnablementPolicy::ENABLED ||
                policy == SpaceObject::EnablementPolicy::NO_ENABLEMENT_POLICY )
        {
            obj["EnablementPolicy"] = SpaceObject::EnablementPolicy::DISABLED;
            this->jsonarray.replace(i, obj);
            QModelIndex element = this->index(i, en_policy_column);
            emit this->dataChanged(element, element);
        }
    }
}


bool SpaceObjectModel::checkMandatoryData(const QJsonValue &value, const QString &type) const
{
    if(type=="TrackPolicy")
    {
        SpaceObject::TrackPolicy policy;
        policy = static_cast<SpaceObject::TrackPolicy>(value.toInt());
        return (policy!=SpaceObject::TrackPolicy::NO_TRACK_POLICY);
    }
    else if(type=="EnablementPolicy")
    {
        SpaceObject::EnablementPolicy policy;
        policy = static_cast<SpaceObject::EnablementPolicy>(value.toInt());
        return (policy!=SpaceObject::EnablementPolicy::NO_ENABLEMENT_POLICY);
    }
    else if ("Amplification" == type)
    {
        return value.toInt() > 0 && value.toInt() <= 100;
    }
    else
    {
        return JsonTableModel::checkMandatoryData(value, type);
    }
}


QVariant SpaceObjectModel::jsonvalueToQvariant(const QJsonValue &value, const QString &type, int role) const
{
    if ("TrackPolicy" == type && value.isDouble())
    {
        auto iterator = SpaceObject::TrackPolicyStringMap.find
                (static_cast<SpaceObject::TrackPolicy>(value.toInt()));
        if (iterator != SpaceObject::TrackPolicyStringMap.end())
        {
            return QVariant(iterator.value());
        }
        else
        {
            return QVariant();
        }
    }
    else if ("EnablementPolicy" == type && value.isDouble())
    {
        auto iterator = SpaceObject::EnablementPolicyStringMap.find
                (static_cast<SpaceObject::EnablementPolicy>(value.toInt()));
        if (iterator != SpaceObject::EnablementPolicyStringMap.end())
        {
            return QVariant(iterator.value());
        }
        else
        {
            return QVariant();
        }
    }
    else
        return JsonTableModel::jsonvalueToQvariant(value, type, role);
}


SpaceObjectModel::ErrorEnum SpaceObjectModel::setExtraParameters(const QStringList &extraparameters)
{
    ErrorEnum error = ErrorEnum::NOT_ERROR;
    m_extraparameters.clear();
    for (const auto& parameter : extraparameters)
    {
        int column = findColumnSectionByIndex(parameter);
        if (-1 != column)
            m_extraparameters[parameter] = column;
    }
    return error;
}


const SpaceObjectModel::ExtraParametersMap &SpaceObjectModel::getExtraParameters() const
{
    return m_extraparameters;
}


QVariant SpaceObjectModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (Qt::CheckStateRole == role)
    {
        if (index.column() >= 0 && index.column() < columnCount() && index.row() >= 0 && index.row() < rowCount())
        {
            const QString& key = header[index.column()]["Index"];
            if(key=="EnablementPolicy")
            {
                QJsonObject obj = this->jsonarray[index.row()].toObject();
                QJsonValue v = obj[key];
                SpaceObject::EnablementPolicy policy = static_cast<SpaceObject::EnablementPolicy>(v.toInt());
                if(policy == SpaceObject::EnablementPolicy::ENABLED ||
                   policy == SpaceObject::EnablementPolicy::ALWAYS_ENABLED)
                {
                    result = Qt::Checked;
                }
                else
                {
                    result = Qt::Unchecked;
                }
            }
        }
    }
    else
    {
        result = JsonTableModel::data(index, role);
    }
    return result;
}


bool SpaceObjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;

    if (role == Qt::CheckStateRole)
    {

        if (index.column() >= 0 && index.column() < header.size() &&
            "EnablementPolicy" == header[index.column()]["Index"] &&
            index.row() >= 0 && index.row() < rowCount())
        {
            QJsonObject current_object = jsonarray[index.row()].toObject();
            SpaceObject::EnablementPolicy current_policy =
                    static_cast<SpaceObject::EnablementPolicy>(current_object["EnablementPolicy"].toInt());

            auto change_enablement_policy = [&](SpaceObject::EnablementPolicy target_policy){
                current_object["EnablementPolicy"] = target_policy;
                jsonarray.replace(index.row(), current_object);
                emit dataChanged(index, index);
            };

            if (SpaceObject::EnablementPolicy::ENABLED == current_policy)
            {
                change_enablement_policy(SpaceObject::EnablementPolicy::DISABLED);
                result = true;
            }
            else if (SpaceObject::EnablementPolicy::DISABLED == current_policy)
            {
                change_enablement_policy(SpaceObject::EnablementPolicy::ENABLED);
                result = true;
            }
        }
    }
    else
    {
        result = JsonTableModel::setData(index, value, role);
    }
    return result;
}


Qt::ItemFlags SpaceObjectModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    else if (index.column() >= 0 && index.column() < columnCount() &&
             "EnablementPolicy" == header[index.column()]["Index"] && this->user_checkable)
    {
        return  Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    }
    else
    {
        return QAbstractTableModel::flags(index);
    }
}


SpaceObjectsList SpaceObjectModel::getObjectsList() const
{
    SpaceObjectsList list;
    QStringList keys = this->getExtraParameters().keys();
    for(const auto& object : this->jsonarray)
    {
        SpaceObject* satellite = new SpaceObject(object, keys);
        list.append(std::make_shared<SpaceObject>(*satellite));
    }
    return list;
}


SpaceObjectsMap SpaceObjectModel::getObjectsMap() const
{
    SpaceObjectsMap map;
    QStringList keys = this->getExtraParameters().keys();
    for(const auto& object : this->jsonarray)
    {
        SpaceObject* satellite = new SpaceObject(object, keys);
        map.insert(satellite->getNorad(), std::make_shared<SpaceObject>(*satellite));
    }
    return map;
}
