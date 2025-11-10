#pragma once

#include <QObject>
#include <QMessageBox>

#include "spcore_global.h"

class SP_CORE_EXPORT SalaraInformation
{

public:

    enum MessageTypeEnum
    {
        CRITICAL = QMessageBox::Critical,
        WARNING = QMessageBox::Warning,
        INFO = QMessageBox::Information
    };

    typedef QPair<int, QString> ErrorPair;
    typedef QList<ErrorPair> ErrorList;

    SalaraInformation(const ErrorPair& error_pair, const QString& detailed = ""):
        error_list({error_pair}), detailed(detailed){}
    SalaraInformation(const ErrorList& error_list):error_list(error_list){}
    SalaraInformation() = default;
    SalaraInformation(const SalaraInformation&) = default;
    SalaraInformation(SalaraInformation&&) = default;
    SalaraInformation& operator =(SalaraInformation&&) = default;
    SalaraInformation& operator =(const SalaraInformation&) = default;

    inline void append(const SalaraInformation& other) {this->error_list.append(other.error_list);}

    bool containsError(int error_code) const;

    inline bool hasError() const {return  !error_list.isEmpty();}
    inline const ErrorList& getErrors() const {return  error_list;}

    void showErrors(const QString &box_title = "", MessageTypeEnum type = INFO,
                    const QString &error_text = "", QWidget* parent = nullptr) const;

    // Static methods.
    static void showError(const QString &box_title = "", const QString& error ="", const QString& detailed = "",
                          MessageTypeEnum type = INFO, QWidget *parent = nullptr);

    static void showInfo(const QString &box_title = "", const QString& info ="", const QString &detailed = "",
                         QWidget *parent = nullptr);
    static void showWarning(const QString &box_title = "", const QString& warning ="", const QString& detailed = "",
                         QWidget *parent = nullptr);
    static void showCritical(const QString &box_title = "", const QString& warning ="", const QString& detailed = "",
                         QWidget *parent = nullptr);

private:
    ErrorList error_list;
    QString detailed;
};
