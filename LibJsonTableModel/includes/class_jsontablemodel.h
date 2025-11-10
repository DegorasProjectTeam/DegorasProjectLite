#pragma once

#include <QAbstractTableModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QSortFilterProxyModel>
#include <QSet>

#include "libjsontablemodel_global.h"

class LIBJSONTABLEMODEL_EXPORT JsonTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ErrorEnum
    {
        NOT_ERROR,
        ROW_OUT_OF_BOUNDS,
        COLUMN_OUT_OF_BOUNDS,
        VALUE_NOT_UNIQUE,
        MANDATORY_VALUE_EMPTY
    };

    enum QVariantModeEnum
    {
        DISPLAY_ROLE = Qt::DisplayRole,
        EDIT_ROLE = Qt::EditRole,
        DIRECT_ROLE = Qt::UserRole,
        OBJECT_ROLE  = Qt::UserRole+1
    };

    static const QMap<Qt::DayOfWeek, QString> name_of_day;
    static const QMap<ErrorEnum, QString> ErrorEnumStringMap;

    typedef QList<QPair<ErrorEnum, QString>> ErrorList;
    typedef QList<QPair<ErrorEnum, QModelIndex>> ErrorIndexList;

    // Heading maps.
    typedef QMap<QString,QString> HeadingMap;
    typedef QVector<HeadingMap> HeaderList;


    JsonTableModel(QObject * parent = nullptr);

    /**
     * @brief Set JsonArray containing the scheme of the table. Each column must contain the following fields:
     *  Index(string) - The name of the field in the json data.
     *  Title(string) - The title in the column of the view.
     *  Tooltip(string) - A tooltip for the column in the view.
     *  Type(string) - A type used in converssions from/to QVariant.
     *  SizeHint(string) - A width hint for the column in the view.
     *  Unique(bool) - Flag to indicate if this field must be unique.
     *  Mandatory(bool) - Flag to indicate if this field needs a mandatory format.
     *  Default(any type of value) - The default value used when a new object is created.
     *
     * If this method is overriden, and the fields above are changed, checkEditedRowIntegrity, checkDataIntegrity,
     * getSizeHint, data and setData methods must be overriden according to the new schema type.
     * @param scheme
     * @return
     */
    virtual ErrorEnum setJsonScheme(const QJsonArray& scheme);

    ErrorIndexList setJsonData(const QJsonDocument& jsondocument);
    ErrorIndexList setJsonData(const QJsonArray& jsonarray);

    // Methods for QAbstractTableModel
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Find methods.
    int findColumnSectionByHeaderProperty(const QString& property, const QString& value) const;
    int findColumnSectionByIndex(const QString &index) const;

    // Getter methods.
    virtual QJsonArray getJsonarray() const;
    virtual QJsonObject getJsonObject(int row ) const;
    virtual bool isJsonSchemeValid() const;

    virtual int getSizeHintForColumn(int column) const;

    ErrorList submitEditedRow(int row);
    void revertEditedRow(int row);

    // Clear method.
    void clearContents();

protected:
    // Temporary object map for store the edited non submited objects.
    typedef QMap<int, QJsonObject> EditedObjectsMap;

    // Complete functionality, but can be overloaded for custom purposes.
    virtual ErrorList checkEditedRowIntegrity(int row) const;
    virtual ErrorIndexList checkDataIntegrity() const;

    // Non abstract. Can be overloaded for adding custom types.
    virtual bool checkMandatoryData(const QJsonValue &value, const QString &type) const;
    virtual QJsonValue QvariantToJsonvalue(const QVariant& variant, const QString& type) const;
    virtual QVariant jsonvalueToQvariant(const QJsonValue &value, const QString &type, int role = DISPLAY_ROLE) const;

    // Class members.
    HeaderList header;
    QJsonArray jsonarray;
    EditedObjectsMap edited_rows;
    QJsonObject json_emtpy_object;
};

class LIBJSONTABLEMODEL_EXPORT JsonTableSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    typedef QHash<int, QString> AppliedFilters;

    explicit JsonTableSortFilterProxyModel(QObject *parent = nullptr);
    explicit JsonTableSortFilterProxyModel(const AppliedFilters& applied_filters, QObject *parent = nullptr);
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

public slots:
    void setFilter(int filter_id, const QString& filter_string = "");
    void unsetFilter(int filter_id);
    void setMandatoryHide(const QSet<int>& rows);
    void setMandatoryHide(int row);
    void unsetMandatoryHide(int row);
    void clearMandatoryHide();

private:
    AppliedFilters m_applied_filters;
    QSet<int> m_mandatory_hide_rows;
};

