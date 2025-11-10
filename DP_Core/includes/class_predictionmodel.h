#pragma once

#include <QSortFilterProxyModel>

#include "class_jsontablemodel.h"
#include "class_spaceobjectmodel.h"
#include "class_prediction.h"
#include "spcore_global.h"

class SP_CORE_EXPORT PredictionModel : public JsonTableModel
{
public:

    PredictionModel(QObject * parent = nullptr);

    QVariant data(const QModelIndex &index, int role = JsonTableModel::DISPLAY_ROLE) const override;
    QList<Prediction*> getObjectsList(const SpaceObjectsMap &map) const;
    Prediction* getRowObject(int row, const std::shared_ptr<SpaceObject> &object) const;

private:
    QVariant jsonvalueToQvariant(const QJsonValue &value, const QString &type,
                                 int role = JsonTableModel::DISPLAY_ROLE) const override;
    bool checkMandatoryData(const QJsonValue &value, const QString &type) const override;
};
