#ifndef FORMSATELLITE_H
#define FORMSATELLITE_H

#include <QWidget>
#include <QDialog>
#include <QVariant>
#include <QMetaType>
#include <QStyledItemDelegate>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

#include "class_spaceobject.h"
#include "interface_spaceobjectsearchengine.h"


class ExtraParamsEventFilter: public QObject
{
    Q_OBJECT
public:
    ExtraParamsEventFilter(QObject* parent):QObject(parent) {}
    ~ExtraParamsEventFilter() {
    }
protected:
    bool eventFilter(QObject* object, QEvent* event)
    {
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_Escape)
            {
                ke->ignore();
                return true;
            }
        }
        if(event->type() == QEvent::Close)
        {
            event->ignore();
            return true;
        }
        return QObject::eventFilter(object, event);
    }
};

namespace Ui{
class FormSatellite;
}

class SpaceObjectModel;
class QDataWidgetMapper;

class FormSatellite : public QDialog
{
    Q_OBJECT

public:

    explicit FormSatellite(const QList<SpaceObjectSearchEngine*>& plugins, SpaceObjectModel* model, int index = 0,
                           QWidget *parent = nullptr);

    virtual ~FormSatellite() override;

    void clearGUI();

public slots:
    void slotSetCurrentSpaceObject(int index);
    void slotCloseReject();


private:
    void autoCompleteFields(const SpaceObject& object);
    void checkIntegrity();

    void searchSpaceObjectData();
    void saveToSalaraSpaceObject();

    void initEditExtraParametersDialog();
    void fillExtraParametersField();

    Ui::FormSatellite *ui;
    SpaceObject object;
    SpaceObject* space_object;
    SpaceObjectModel* m_model;
    QDataWidgetMapper* m_mapper;
    QDataWidgetMapper* m_extraparameters_mapper;
    QDialog* form_extraparameters;
    QList<SpaceObjectSearchEngine*> list_plugin;
    QString last_norad;
    QMap<QString, QLineEdit*> m_extraparameters_fields;

    bool eventFilter(QObject *obj, QEvent *event) override;
};

class SpaceObjectDelegate : public QStyledItemDelegate
{
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

#endif // FORM_SATELLITE_H
