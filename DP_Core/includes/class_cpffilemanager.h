#pragma once

#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QDir>
#include <QObject>
#include <QMap>
#include <QDate>
#include <QDateTime>

#include <memory>

#include "class_spaceobject.h"
#include "class_spaceobjectmodel.h"
#include "class_cpf.h"
#include "class_prediction.h"
#include "class_salarainformation.h"
#include "spcore_global.h"

struct SP_CORE_EXPORT CPFSelected
{
    inline CPFSelected():space_object(nullptr), cpf(nullptr),
        calculation_days(0), remaining_days(0), total_days(0), valid(false){}
    inline CPFSelected(const std::shared_ptr<SpaceObject>& object, const std::shared_ptr<CPF> cpf,
                         double c_days, double r_days, double t_days):
        space_object(object), cpf(cpf), calculation_days(c_days),
        remaining_days(r_days), total_days(t_days), valid(true){}
    std::shared_ptr<SpaceObject> space_object;
    std::shared_ptr<CPF> cpf;
    double calculation_days;
    double remaining_days;
    double total_days;
    bool valid;
};

class SP_CORE_EXPORT CPFFileManager : public QObject
{
    Q_OBJECT
public:

    enum ErrorEnum
    {
        CPF_NOT_FOUND=1,
        CPF_LOAD_FAILED=2,
        CPF_INVALID=3,
        CPF_OLD
    };

    enum SelectionOption
    {
        MOST_CURRENT,
        MAXIMIZE_DAYS,
        MOST_CURRENT_REDUCE,
        MAXIMIZE_DAYS_REDUCE,
        MOST_CURRENT_FIXED,
        SELECTION_END
    };

    enum ProviderOption
    {
        PREFERRED,
        ALL,
        CUSTOM
    };

    enum PriorityTLE
    {
        NORMAL_PRIORITY=0,
        LOWEST_PRIORITY=1
    };

    enum ForceProviderOption
    {
        NO_FORCE = 0,
        FORCE = 1
    };

    static const QMap<SelectionOption, QString> kMapSelectionString;
    static const QMap<ProviderOption, QString> kMapProviderString;

    // Static class. Delete constructor.
    CPFFileManager() = delete;

    // Load methods.
    static SalaraInformation loadMultipleCPF(const QString& path_data, const SpaceObjectsList &object_list,
                                             const QDateTime &start, const QDateTime &end,
                                             SelectionOption selection_option, ProviderOption provider_option,
                                             ForceProviderOption force_provider, PriorityTLE tle_prior,
                                             QList<CPFSelected> &cpf_list,
                                             const QString& provider="");

    static SalaraInformation loadSingleCPF(const QString& path_data, const SpaceObject &object, const QDateTime &start,
                                           const QDateTime &end, SelectionOption selection_option,
                                           ProviderOption provider_option, ForceProviderOption force_provider,
                                           PriorityTLE tle_prior, std::shared_ptr<CPF> &cpf, double& t_days,
                                           double& c_days, double &r_days, const QString& provider="");

    static SalaraInformation loadSingleCPFRecursive(const QString& path_data, const SpaceObject &object,
                                                    const QDateTime &start, const QDateTime &end,
                                                    SelectionOption selection_option, ProviderOption provider_option,
                                                    ForceProviderOption force_provider, PriorityTLE tle_prior,
                                                    std::shared_ptr<CPF> &cpf, double& t_days, double& c_days,
                                                    double &r_days, const QString& provider="",
                                                    unsigned int days_before = 6);

    static QStringList getCPFFilesForObject(const QString& path_data, const SpaceObject& object);

    static QStringList getCPFFilePathsForTracking(const QString &path_data, const QDateTime &start_time,
                                                  const QDateTime &end_time, const SpaceObject &object);

    static QString findCPF(const QString &cpf_name, int days_after = 20);

private:
    static SalaraInformation privateLoadCPF(const QString& path_data, const SpaceObject& object,
                                            const QDateTime& start, const QDateTime& end,
                                            SelectionOption selection_option, ProviderOption provider_option,
                                            PriorityTLE tle_prior, std::shared_ptr<CPF> &cpf_final,
                                            double& t_days, double& c_days, double& r_days, const QString& provider);
};

Q_DECLARE_METATYPE(CPFSelected)
Q_DECLARE_METATYPE(CPFSelected*)
