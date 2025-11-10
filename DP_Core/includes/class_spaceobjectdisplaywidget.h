#pragma once

#include "spcore_global.h"

#include <QWidget>

namespace Ui {
class SpaceObjectDisplayWidget;
}

class JsonTableSortFilterProxyModel;
class SpaceObjectsModelLoader;
class SpaceObjectModel;

class SP_CORE_EXPORT SpaceObjectDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpaceObjectDisplayWidget(QWidget *parent = nullptr);
    ~SpaceObjectDisplayWidget();


    QList<QAction *> getEditActions() const;
    QString getSystemSetName() const;
    QString getVersionName() const;
    QDateTime getVersionTime() const;

private:

    Ui::SpaceObjectDisplayWidget *ui;

    void loadModel();

    JsonTableSortFilterProxyModel* spaceobject_sortmodel;
    SpaceObjectModel* spaceobject_model;

};

