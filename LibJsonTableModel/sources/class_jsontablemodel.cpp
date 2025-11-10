#include "includes/class_jsontablemodel.h"

#include <QJsonDocument>
#include <QDateTime>

const QMap<Qt::DayOfWeek, QString> JsonTableModel::name_of_day{{Qt::Monday, "Monday"},
                                         {Qt::Tuesday, "Tuesday"},
                                         {Qt::Wednesday, "Wednesday"},
                                         {Qt::Thursday, "Thursday"},
                                         {Qt::Friday, "Friday"},
                                         {Qt::Saturday, "Saturday"},
                                         {Qt::Sunday, "Sunday"}};

const QMap<JsonTableModel::ErrorEnum, QString> JsonTableModel::ErrorEnumStringMap = {
    {JsonTableModel::ErrorEnum::NOT_ERROR, "No Policy"},
    {JsonTableModel::ErrorEnum::ROW_OUT_OF_BOUNDS, "Row out of bounds"},
    {JsonTableModel::ErrorEnum::COLUMN_OUT_OF_BOUNDS, "Column out of bounds"},
    {JsonTableModel::ErrorEnum::VALUE_NOT_UNIQUE, "Unique value is already used"},
    {JsonTableModel::ErrorEnum::MANDATORY_VALUE_EMPTY, "Mandatory value is not valid"}
};

JsonTableModel::JsonTableModel(QObject *parent) : QAbstractTableModel (parent)
{
}

JsonTableModel::ErrorIndexList JsonTableModel::checkDataIntegrity() const
{
    JsonTableModel::ErrorIndexList error_list;

    int i = 0;
    for (auto it = jsonarray.constBegin(); it != jsonarray.constEnd(); it++)
    {
        QJsonObject object = it->toObject();

        int j = 0;
        // Check mandatory format and uniqueness for each column in the row
        for (const auto& column : header)
        {
            QString column_index = column["Index"];

            if (column.contains("Mandatory"))
            {
                auto type = column.find("Type");
                if (!checkMandatoryData(object[column_index], type.value()))
                {
                    error_list.push_back(qMakePair(
                                             JsonTableModel::ErrorEnum::MANDATORY_VALUE_EMPTY, this->index(i, j)));
                }
            }

            if (column.contains("Unique"))
            {
                bool unique = true;

                // If value is not an empty string, check if it is unique
                if (!(object[column_index].isString() && object[column_index].toString().isEmpty()))
                {
                    // Check uniqueness of the cell with the previous rows
                    for (auto other = jsonarray.constBegin(); other != it && unique; other++)
                    {
                        QJsonObject other_object = other->toObject();
                        if (other_object[column_index] == object[column_index])
                        {
                            unique = false;
                            error_list.push_back(qMakePair(
                                                     JsonTableModel::ErrorEnum::VALUE_NOT_UNIQUE, this->index(i,j)));
                        }
                    }
                }
            }
            j++;
        }
        i++;
    }

    return error_list;
}

bool JsonTableModel::checkMandatoryData(const QJsonValue &value, const QString &type) const
{
    if ((type == "Date" || type == "Time" || type == "WeekDay" || type == "DateTime"))
    {
        if (value.isString() && !value.toString().isEmpty())
        {
             return QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss").isValid();
        }
        else
        {
            return false;
        }

    }
    else if(type=="BoolTristate" )
    {
        if (value.isDouble())
            return value.toInt() <= 2 && value.toInt() >= 0;
        else
            return  false;
    }
    else if(type=="NotDisplayZero")
    {
        return value.isDouble();
    }
    else if (value.isString())
    {
        return !value.toString().isEmpty();
    }
    else
    {
        return true;
    }
}

QJsonValue JsonTableModel::QvariantToJsonvalue(const QVariant &variant, const QString &type) const
{
    Q_UNUSED(type)
    return QJsonValue::fromVariant(variant);
}


QVariant JsonTableModel::jsonvalueToQvariant(const QJsonValue &value, const QString &type, int role) const
{
    if(role == JsonTableModel::DIRECT_ROLE)
        return value.toVariant();


    else if(type == "NumericID" && value.isString() && !value.toString().isEmpty())
    {
        return value.toString().toInt();
    }
    else if (type == "WeekDay" && value.isString() && !value.toString().isEmpty())
    {
        QDateTime aux = QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss");
        return name_of_day[static_cast<Qt::DayOfWeek>(aux.date().dayOfWeek())];
    }
    else if (type == "Date" && value.isString() &&  !value.toString().isEmpty())
    {
        if(role==DISPLAY_ROLE)
            return QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss").toString("dd-MM-yyyy");
        else if(role==OBJECT_ROLE)
            return QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss").date();
        else
            return QVariant();
    }
    else if (type == "Time" && value.isString() && !value.toString().isEmpty())
    {
        if(role==DISPLAY_ROLE)
            return QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss").toString("hh:mm");
        else if(role==OBJECT_ROLE)
            return QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss").time();
        else
            return QVariant();
    }
    else if (type == "DateTime" && value.isString() && !value.toString().isEmpty())
    {
        if(role==DISPLAY_ROLE)
            return QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss").toString("dd-MM-yyyy hh:mm");
        else if(role==OBJECT_ROLE)
            return QDateTime::fromString(value.toString(), "yyyyMMdd.hhmmss");
        else
            return QVariant();
    }
    else if(type == "BoolTristate" && value.isDouble())
    {
        if(role==DISPLAY_ROLE)
        {
            if(value.toInt()==0)
                return "No";
            else if (value.toInt()==1)
                return "Yes";
            else
                return "Unknown";
        }else if(role==OBJECT_ROLE)
            return value.toInt();
        else
            return QVariant();
    }
    else if (type == "Bool" && value.isDouble())
    {
        if(role==DISPLAY_ROLE)
        {
            if(value.toBool())
                return "Yes";
            else
                return "No";
        }
        else if(role==OBJECT_ROLE)
            return value.toBool();
        else
            return QVariant();
    }
    else if (type == "NotDisplayZero" && value.isDouble())
    {
        if (0.0 != value.toDouble())
        {
            return value.toDouble();
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        return value.toVariant();
    }
}

JsonTableModel::ErrorList JsonTableModel::checkEditedRowIntegrity(int row) const
{

    JsonTableModel::ErrorList error_list;
    auto it = edited_rows.find(row);

    // If there is edited data for the row
    if (it != edited_rows.end())
    {
        const QJsonObject& object = it.value();

        // Check mandatory format and uniqueness for each column in the row
        for (const auto& column : header)
        {
            QString column_index = column["Index"];

            if (column.contains("Mandatory"))
            {
                auto type = column.find("Type");
                if (!checkMandatoryData(object[column_index], type.value()))
                {
                    error_list.push_back(qMakePair(
                                             JsonTableModel::ErrorEnum::MANDATORY_VALUE_EMPTY, column_index));
                }
            }

            if (column.contains("Unique"))
            {
                bool unique = true;
                int other_row = 0;

                // If value is not an empty string, check if it is unique
                if (!(object[column_index].isString() && object[column_index].toString().isEmpty()))
                {
                    while ( unique && other_row < rowCount())
                    {
                        if (row != other_row &&
                                jsonarray[other_row].toObject()[column_index] == object[column_index])
                        {
                            unique = false;
                            error_list.push_back(qMakePair(
                                                     JsonTableModel::ErrorEnum::VALUE_NOT_UNIQUE, column["Index"]));
                        }
                        else
                        {
                            other_row++;
                        }
                    }
                }
            }
        }
    }

    return error_list;
}

// TODO: This method, and the whole submitting and checking should be changed to work with cells instead of rows.
// That way, there wouldn't be extra checkings and only data that has actually been changed would be managed.
// This change implies changes setData, and checkIntegrityEditedRow too.
JsonTableModel::ErrorList JsonTableModel::submitEditedRow(int row)
{
    ErrorList errors;
    // If there is edited data for the row, check integrity.
    // If checking is successful, submit data. Otherwise return errors
    if (edited_rows.contains(row))
    {
        errors = checkEditedRowIntegrity(row);

        if (errors.isEmpty())
        {
            jsonarray.replace(row, edited_rows[row]);
            edited_rows.remove(row);
            emit dataChanged(index(row, 0), index(row, columnCount() - 1));
        }
    }
    return errors;
}

void JsonTableModel::revertEditedRow(int row)
{
    edited_rows.remove(row);
    emit dataChanged(index(row, 0), index(row, columnCount() - 1),{Qt::EditRole});
}

int JsonTableModel::getSizeHintForColumn(int column) const
{
    if (column >= 0 && column < columnCount())
    {
        auto size_hint = header[column].find("SizeHint");

        if (size_hint != header[column].end())
        {
            bool conversion_result = false;
            int result = size_hint.value().toInt(&conversion_result);

            if (conversion_result)
            {
                return result;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return  -1;
        }
    }
    else
    {
        return -1;
    }
}

int JsonTableModel::findColumnSectionByHeaderProperty(const QString &property, const QString &value) const
{
    int i = 0;
    bool found = false;
    if (i < columnCount() && header[i].contains(property))
    {
        while (i < columnCount() && !found)
        {
            if (value == header[i][property])
                found = true;
            else
                i++;
        }
    }
    return found ? i : -1;
}

int JsonTableModel::findColumnSectionByIndex(const QString &index) const
{
    return findColumnSectionByHeaderProperty("Index", index);
}

QJsonArray JsonTableModel::getJsonarray() const
{
    return this->jsonarray;
}

QJsonObject JsonTableModel::getJsonObject(int row) const
{
    return  this->jsonarray[row].toObject();
}

bool JsonTableModel::isJsonSchemeValid() const
{
    return !this->header.isEmpty();
}


JsonTableModel::ErrorIndexList JsonTableModel::setJsonData(const QJsonDocument& jsondocument)
{
    return this->setJsonData(jsondocument.array());
}

JsonTableModel::ErrorIndexList JsonTableModel::setJsonData(const QJsonArray& jsonarray)
{
    this->beginResetModel();
    this->jsonarray = jsonarray;
    ErrorIndexList error_list = checkDataIntegrity();
    if(!error_list.isEmpty())
        this->clearContents();
    this->endResetModel();
    return error_list;
}

void JsonTableModel::clearContents()
{
    this->beginResetModel();
    this->jsonarray = QJsonArray();
    this->endResetModel();
}

JsonTableModel::ErrorEnum JsonTableModel::setJsonScheme(const QJsonArray &scheme)
{
    // Clean
    this->header.clear();
    // Set the header using the scheme.
    for(const auto& value : scheme)
    {
        QString title = value["Title"].toString();
        QString index = value["Index"].toString();
        QString tooltip = value["ToolTip"].toString();
        QString type = value["Type"].toString();
        QString size = value["SizeHint"].toString();
        bool unique = value["Unique"].toBool();
        bool mandatory = value["Mandatory"].toBool();
        this->header.push_back(JsonTableModel::HeadingMap({{"Title",title}, {"Index",index},{"ToolTip",tooltip},
                                                             {"Type", type}, {"SizeHint", size}}));
        if (unique)
        {
            this->header.back().insert("Unique", "true");
        }

        if (mandatory)
        {
            this->header.back().insert("Mandatory", "true");
        }

        QJsonValue def = value["Default"];
        if (def.isUndefined())
        {
            this->json_emtpy_object.insert(index, "");
        }
        else
        {
            this->json_emtpy_object.insert(index, value["Default"]);
        }
    }
    this->setJsonData(QJsonArray());
    return ErrorEnum::NOT_ERROR;
}

QVariant JsonTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result;

    if (section < 0 || section >= this->header.size())
        return result;

    if( role != Qt::DisplayRole && role != Qt::ToolTipRole )
        return result;

    switch( orientation )
    {
    case Qt::Horizontal:
        if (Qt::DisplayRole == role)
        {
            result = header[section]["Title"];
        }
        else if (Qt::ToolTipRole == role)
        {
            result = header[section]["ToolTip"];
        }
        break;
    case Qt::Vertical:
        break;
    }
    return result;
}

int JsonTableModel::rowCount(const QModelIndex&) const
{
    return jsonarray.size();
}

int JsonTableModel::columnCount(const QModelIndex&) const
{
    return header.size();
}

QVariant JsonTableModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case JsonTableModel::DIRECT_ROLE:
    {
        if (index.column() >= 0 && index.column() < columnCount() && index.row() >= 0 && index.row() < rowCount())
        {
            QJsonObject obj = jsonarray[index.row()].toObject();
            const QString& key = header[index.column()]["Index"];
            if(obj.contains(key))
                return QVariant(obj[key]);

        }
        break;
    }
    case JsonTableModel::DISPLAY_ROLE:
    {
        if (index.column() >= 0 && index.column() < columnCount() && index.row() >= 0 && index.row() < rowCount())
        {
            QJsonObject obj = jsonarray[index.row()].toObject();
            const QString& key = header[index.column()]["Index"];
            if( obj.contains( key ))
                return jsonvalueToQvariant(obj[key], header[index.column()]["Type"], JsonTableModel::DISPLAY_ROLE);
        }
        break;
    }
    case JsonTableModel::OBJECT_ROLE:
    {
        if (index.column() >= 0 && index.column() < columnCount() && index.row() >= 0 && index.row() < rowCount())
        {
            QJsonObject obj = jsonarray[index.row()].toObject();
            const QString& key = header[index.column()]["Index"];
            if( obj.contains( key ))
            {
                QJsonValue v = obj[ key ];
                return jsonvalueToQvariant(v, header[index.column()]["Type"], JsonTableModel::OBJECT_ROLE);
            }
        }
        break;
    }
    case JsonTableModel::EDIT_ROLE:
    {
        // Select edited object if it exists, or stored object otherwise
        QJsonObject obj;
        auto edited_object = edited_rows.find(index.row());
        if (edited_object != edited_rows.end())
        {
            obj = edited_object.value();
        }
        else if (index.row() >= 0 && index.row() < rowCount()) {
            obj = jsonarray[index.row()].toObject();
        }

        if (index.column() >= 0 && index.column() < columnCount() && !obj.isEmpty())
        {

            const QString& key = header[index.column()]["Index"];
            if( obj.contains( key ))
            {
                QJsonValue v = obj[ key ];
                return jsonvalueToQvariant(v, header[index.column()]["Type"], JsonTableModel::DISPLAY_ROLE);
            }
        }
        break;
    }
    }
    return QVariant();
}

bool JsonTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid())
    {
        switch(role)
        {
            case JsonTableModel::EDIT_ROLE:
            {
                QJsonObject obj;
                auto edited_object = edited_rows.find(index.row());
                if (edited_object != edited_rows.end())
                {
                    obj = edited_object.value();
                }
                else
                {
                    obj = jsonarray[index.row()].toObject();
                }

                const QString& key = header[index.column()]["Index"];
                obj[key] = QvariantToJsonvalue(value, header[index.column()]["Type"]);
                edited_rows.insert(index.row(), obj);

                emit dataChanged(index, index, {Qt::EditRole});

                return true;
            }
            case JsonTableModel::DIRECT_ROLE:
            {
                QJsonObject obj = jsonarray[index.row()].toObject();

                const QString& key = header[index.column()]["Index"];
                if(obj.contains(key))
                {
                    obj[key] = QvariantToJsonvalue(value, header[index.column()]["Type"]);
                    jsonarray.replace(index.row(), obj);

                    emit dataChanged(index, index);

                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
    return false;
}

bool JsonTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    this->beginInsertRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; i++)
    {
        this->jsonarray.insert(i, json_emtpy_object);
    }
    this->endInsertRows();
    return true;
}

bool JsonTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    this->beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; i++)
    {
        this->jsonarray.takeAt(i);  // Qt Bug. We cant use remove. Ask Angel Vera or Jesus Relinque.
    }
    this->endRemoveRows();
    return true;
}

JsonTableSortFilterProxyModel::JsonTableSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel (parent)
{

}

JsonTableSortFilterProxyModel::JsonTableSortFilterProxyModel(
        const JsonTableSortFilterProxyModel::AppliedFilters &applied_filters, QObject *parent) :
    QSortFilterProxyModel(parent),m_applied_filters(applied_filters)
{

}

bool JsonTableSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(this->m_mandatory_hide_rows.contains(source_row))
        return false;

    bool accept = true;

    AppliedFilters::const_iterator it = m_applied_filters.cbegin();

    while (it != m_applied_filters.end() && accept)
    {
        QModelIndex column_index = sourceModel()->index(source_row, it.key(), source_parent);
        QVariant value = sourceModel()->data(column_index);
        if (value.canConvert<QString>())
        {
            accept = accept && value.toString().contains(QRegularExpression(it.value()));
        }
        it++;
    }

    return accept && QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void JsonTableSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QObject::disconnect(this);

    QObject::connect(sourceModel, &QAbstractItemModel::rowsAboutToBeMoved,
                     this, &JsonTableSortFilterProxyModel::clearMandatoryHide);
    QObject::connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                     this, &JsonTableSortFilterProxyModel::clearMandatoryHide);
    QObject::connect(sourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
                     this, &JsonTableSortFilterProxyModel::clearMandatoryHide);
    QObject::connect(sourceModel, &QAbstractItemModel::modelAboutToBeReset,
                     this, &JsonTableSortFilterProxyModel::clearMandatoryHide);

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void JsonTableSortFilterProxyModel::setFilter(int filter_id, const QString &filter_string)
{
    this->m_applied_filters[filter_id] = filter_string;
    this->invalidateFilter();
}

void JsonTableSortFilterProxyModel::unsetFilter(int filter_id)
{
    this->m_applied_filters.remove(filter_id);
    this->invalidateFilter();
}

void JsonTableSortFilterProxyModel::setMandatoryHide(const QSet<int> &rows)
{
    this->m_mandatory_hide_rows = rows;
    this->invalidateFilter();
}

void JsonTableSortFilterProxyModel::setMandatoryHide(int source_row)
{
    if(!this->m_mandatory_hide_rows.contains(source_row))
    {
        this->m_mandatory_hide_rows.insert(source_row);
        this->invalidateFilter();
    }
}

void JsonTableSortFilterProxyModel::unsetMandatoryHide(int source_row)
{
    bool removed = this->m_mandatory_hide_rows.remove(source_row);
    if(removed)
        this->invalidateFilter();
}

void JsonTableSortFilterProxyModel::clearMandatoryHide()
{
    this->setMandatoryHide(QSet<int>());
}
