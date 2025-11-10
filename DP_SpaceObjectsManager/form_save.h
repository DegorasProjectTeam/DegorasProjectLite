#pragma once

#include <QDialog>
#include <QDateTime>

namespace Ui {
class FormSave;
}

class FormSave : public QDialog
{
    Q_OBJECT

public:
    explicit FormSave(QString name, QString comment, QString versionname,
                      const QDateTime& versiondate, QWidget *parent = nullptr);
    ~FormSave();

    inline QString getVersionName() const {return this->version;}
    inline const QDateTime& getVersionDate() const {return this->versiontime;}
    inline QString getComment() const {return this->comment;}
    inline QString getName() const {return this->name;}

private:
    Ui::FormSave *ui;
    QString version;
    QString comment;
    QString name;
    QDateTime versiontime;

};

