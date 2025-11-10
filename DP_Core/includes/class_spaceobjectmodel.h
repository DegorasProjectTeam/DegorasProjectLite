#pragma once

#include <QList>
#include <QMap>

#include <memory>

#include "class_jsontablemodel.h"
#include "class_spaceobject.h"
#include "spcore_global.h"

using SpaceObjectsList = QList<std::shared_ptr<SpaceObject>>;
using SpaceObjectsMap = QMap<QString, std::shared_ptr<SpaceObject>>;

class SP_CORE_EXPORT SpaceObjectModel : public JsonTableModel
{
    Q_OBJECT

public:

    // Map for store extra parameters.
    typedef QMap<QString, int> ExtraParametersMap;

    // Constructor.
    SpaceObjectModel(QObject * parent = nullptr, bool user_checkable = true);

    // Override methods.
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = JsonTableModel::EDIT_ROLE) override;

    // Getters.
    SpaceObjectsList getObjectsList() const;
    SpaceObjectsMap getObjectsMap() const;
    const ExtraParametersMap& getExtraParameters() const;

    // Custom setters.
    ErrorEnum setExtraParameters(const QStringList& extraparameters);

    // Custom methods.
    void disableAll();

private:

    // Override methods.
    bool checkMandatoryData(const QJsonValue &value, const QString &type) const override;
    QVariant jsonvalueToQvariant(const QJsonValue &value, const QString &type,
                                 int role = JsonTableModel::DISPLAY_ROLE) const override;

    // Private variables.
    ExtraParametersMap m_extraparameters;
    bool user_checkable;
};


