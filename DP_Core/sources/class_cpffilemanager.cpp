#include "includes/class_salarasettings.h"
#include "includes/class_cpffilemanager.h"
#include "includes/class_globalutils.h"

#include <QDebug>
#include <QtMath>

#include <set>
#include <omp.h>
#include <memory>

const QMap<CPFFileManager::SelectionOption, QString> CPFFileManager::kMapSelectionString {
    {CPFFileManager::SelectionOption::MOST_CURRENT, "Most current"},
    {CPFFileManager::SelectionOption::MAXIMIZE_DAYS, "Maximize days"},
    {CPFFileManager::SelectionOption::MOST_CURRENT_REDUCE, "Most current and reduce"},
    {CPFFileManager::SelectionOption::MAXIMIZE_DAYS_REDUCE, "Maximize days and reduce"},
    {CPFFileManager::SelectionOption::MOST_CURRENT_FIXED, "Most current and fixed"}
};

const QMap<CPFFileManager::ProviderOption, QString> CPFFileManager::kMapProviderString
{
    {CPFFileManager::ProviderOption::ALL, "All"},
    {CPFFileManager::ProviderOption::PREFERRED, "Preferred"},
    {CPFFileManager::ProviderOption::CUSTOM, "Custom"},
};

SalaraInformation CPFFileManager::loadMultipleCPF(const QString& path_data,
                                                  const SpaceObjectsList& object_list,
                                                  const QDateTime &start, const QDateTime &end,
                                                  SelectionOption selection_option,
                                                  ProviderOption provider_option,
                                                  ForceProviderOption force_provider,
                                                  PriorityTLE tle_prior,
                                                  QList<CPFSelected>& cpf_list,
                                                  const QString& provider)
{
    SalaraInformation::ErrorList errors;
    cpf_list.clear();

    #pragma omp parallel for num_threads(omp_get_max_threads()) schedule(dynamic)
    for(int i=0; i<object_list.size(); i++)
    {
        const std::shared_ptr<SpaceObject>& object = object_list[i];
        std::shared_ptr<CPF> cpf_final;
        double t_days, c_days, r_days;
        SalaraInformation single_errors =
                CPFFileManager::loadSingleCPF(path_data, *object, start, end, selection_option, provider_option,
                                                force_provider, tle_prior, cpf_final, t_days, c_days, r_days, provider);
        #pragma omp critical
        {
            if(!single_errors.containsError(CPFFileManager::CPF_NOT_FOUND))
                cpf_list.append(CPFSelected(object, cpf_final, c_days, r_days, t_days));
            errors.append(single_errors.getErrors());
        }
    }

    // Now we must reduce the calculation days if neccesary.
    if(selection_option == CPFFileManager::SelectionOption::MAXIMIZE_DAYS_REDUCE ||
            selection_option == CPFFileManager::SelectionOption::MOST_CURRENT_REDUCE)
    {
        // Aqui buscariamos el CPF con el menor numero de dias que no sean cero.
        double min_days = std::numeric_limits<double>::max();

        for(const auto& cpf_selected : cpf_list)
            if(cpf_selected.remaining_days<min_days)
                min_days = cpf_selected.remaining_days;

        // Actualizamos...
        for(auto&& cpf_selected : cpf_list)
            cpf_selected.calculation_days = min_days;
    }

    // Return errors.
    return SalaraInformation(errors);
}


SalaraInformation CPFFileManager::loadSingleCPF(const QString& path_data, const SpaceObject& object,
                                                const QDateTime& start, const QDateTime& end,
                                                SelectionOption selection_option,
                                                ProviderOption provider_option,
                                                ForceProviderOption force_provider,
                                                PriorityTLE tle_prior,
                                                std::shared_ptr<CPF> &cpf_final,
                                                double& t_days, double& c_days, double& r_days,
                                                const QString& provider)
{
    //Error list.
    SalaraInformation errors;

    // First search.
    errors = CPFFileManager::privateLoadCPF(path_data, object, start, end, selection_option, provider_option,
                                             tle_prior,  cpf_final, t_days, c_days, r_days, provider);

    // Second search if neccesary.
    if(errors.containsError(ErrorEnum::CPF_NOT_FOUND) && (force_provider == ForceProviderOption::NO_FORCE) &&
            (provider_option != ProviderOption::ALL))
    {
        errors = CPFFileManager::privateLoadCPF(path_data, object, start, end, selection_option,
                                                ProviderOption::ALL, tle_prior, cpf_final, t_days, c_days, r_days, "");
    }

    return errors;
}

SalaraInformation CPFFileManager::loadSingleCPFRecursive(const QString &path_data, const SpaceObject &object,
                                                         const QDateTime &start, const QDateTime &end,
                                                         CPFFileManager::SelectionOption selection_option,
                                                         CPFFileManager::ProviderOption provider_option,
                                                         CPFFileManager::ForceProviderOption force_provider,
                                                         CPFFileManager::PriorityTLE tle_prior,
                                                         std::shared_ptr<CPF> &cpf, double &t_days, double &c_days,
                                                         double &r_days, const QString &provider,
                                                         unsigned int days_before)
{
    SalaraInformation result;
    SalaraInformation errors;
    unsigned int i = 0;
    QDate date = end.date();
    std::shared_ptr<CPF> best_tle;

    do
    {
        result.append(CPFFileManager::loadSingleCPF(path_data + '/' + date.toString("yyyyMMdd"), object, start, end,
                                                 selection_option, provider_option, force_provider, tle_prior, cpf,
                                                 t_days, c_days, r_days, provider));
        // If a cpf was found
        if (cpf)
        {
            // If TLE is lowest priority, but TLE was returned, then save the first TLE and continue searching
            if (tle_prior == PriorityTLE::LOWEST_PRIORITY &&
                     "tle" == QString::fromStdString(cpf->getHeader().basicInfo1Header()->cpf_source).toLower())
            {
                // Save only the first valid TLE
                if (!best_tle)
                    best_tle = cpf;

                cpf.reset();
            }
        }

        date = date.addDays(-1);
        i++;

    } while(i < days_before && !cpf);

    // If there is only a TLE as option, choose it
    if (!cpf && best_tle)
        cpf = best_tle;

    return result;

}

QStringList CPFFileManager::getCPFFilesForObject(const QString& path_data, const SpaceObject &object)
{

    QStringList result;
    for (const auto& cpf_path : QDir(path_data).entryList(QDir::Files))
    {
        QString cpf_abspath = path_data + '/' + cpf_path;
        CPF cpf(cpf_abspath.toStdString(), CPF::OpenOptionEnum::ONLY_HEADER);
        if (!cpf.empty() && cpf.getHeader().basicInfo1Header() && cpf.getHeader().basicInfo2Header())
        {
            QDateTime cpf_endtime = GlobalUtils::timePointToQDateTime(cpf.getHeader().basicInfo2Header()->end_time);
            if (cpf.getHeader().basicInfo2Header()->norad == object.getNorad().toStdString() &&
                    cpf_endtime > QDateTime::currentDateTimeUtc())
                result.append(cpf_path);
        }

    }
    return result;
}

QStringList CPFFileManager::getCPFFilePathsForTracking(const QString& path_data, const QDateTime& start_time,
                                                       const QDateTime& end_time, const SpaceObject &object)
{
    QString date_path;
    // TODO: what happens with daily trackings?
    if (start_time.time() < QTime(12,0))
        date_path = start_time.date().addDays(-1).toString("yyyyMMdd");
    else
        date_path = start_time.date().toString("yyyyMMdd");

    QString path = (path_data.isEmpty() ? "." : path_data) + '/' + date_path;
    QStringList result;

    for (const auto& cpf_path : QDir(path).entryList(QDir::Files))
    {
        QString cpf_abspath = path + '/' + cpf_path;
        CPF cpf(cpf_abspath.toStdString(), CPF::OpenOptionEnum::ONLY_HEADER);
        if (!cpf.empty() && cpf.getHeader().basicInfo1Header() && cpf.getHeader().basicInfo2Header())
        {
            QDateTime cpf_starttime = GlobalUtils::timePointToQDateTime(cpf.getHeader().basicInfo2Header()->start_time);
            QDateTime cpf_endtime = GlobalUtils::timePointToQDateTime(cpf.getHeader().basicInfo2Header()->end_time);
            if (cpf.getHeader().basicInfo2Header()->norad == object.getNorad().toStdString() &&
                    start_time >= cpf_starttime && end_time <= cpf_endtime )
                result.append(cpf_abspath);
        }

    }
    return result;
}


SalaraInformation CPFFileManager::privateLoadCPF(const QString& path_data, const SpaceObject& object,
                                                 const QDateTime& start, const QDateTime& end,
                                                 SelectionOption selection_option,
                                                 ProviderOption provider_option,
                                                 PriorityTLE tle_prior,
                                                 std::shared_ptr<CPF> &cpf_final,
                                                 double& t_days, double& c_days, double& r_days,
                                                 const QString& provider)
{
    //Error list.
    SalaraInformation::ErrorList error_list;

    // CPF directory.
    QDir dir_cpfs(path_data);

    //Final ordered cpf list.
    QList<std::shared_ptr<CPF>> list_cpf;

    // Clear the cpf first.
    cpf_final.reset();

    // Filter for search the cpf and file selected string.
    QString filter;

    // Select the cpf extension using the provider option.
    if(provider_option == CPFFileManager::ProviderOption::PREFERRED)
    {
        QString preferred = object.getCPFProvider();
        if(preferred == SpaceObject::kAllCPFProvider || preferred == SpaceObject::kNoTLEProvider)
            filter = "*.*";
        else
            filter = "*."+preferred;
    }
    else if (provider_option == CPFFileManager::ProviderOption::ALL)
        filter = "*.*";
    else if (provider_option == CPFFileManager::ProviderOption::CUSTOM)
        filter = "*."+provider;

    // Update the name filter with the extension.
    dir_cpfs.setNameFilters(QStringList()<<filter);

    // We must open the header of the cpfs for search the norad. I don't understand
    //  why ilrs and other institutions use a "satellite name" for the cpf namefiles...
    //  For example, all SL16 debris have the same name... In our system we will use norad.
    QStringList list_filenames = dir_cpfs.entryList(QDir::Files);
    for(const auto& filename : list_filenames)
    {
        // Open only the header.
        auto cpf = std::make_shared<CPF>((path_data+'/'+filename).toStdString(), CPF::OpenOptionEnum::ONLY_HEADER);

        if(!cpf->empty() && cpf->getHeader().basicInfo1Header() && cpf->getHeader().basicInfo2Header())
        {
            // Prediction end time.
            QDateTime p_end = GlobalUtils::timePointToQDateTime(cpf->getHeader().basicInfo2Header()->end_time);
            // CPF duration.
            std::chrono::seconds duration =
                    std::chrono::duration_cast<std::chrono::seconds>(
                        cpf->getHeader().basicInfo2Header()->end_time - cpf->getHeader().basicInfo2Header()->start_time);



            // Check if the cpf is for the selected object.
            if(cpf->getHeader().basicInfo2Header()->norad == object.getNorad().toStdString())
            {
                // Check if is valid CPF.
                if(cpf->getHeader().basicInfo2Header()->target_class == CPFHeader::TargetClassEnum::PASSIVE_LRR_LUNAR ||
                   cpf->getHeader().basicInfo2Header()->target_class == CPFHeader::TargetClassEnum::SYNC_TRANSPONDER  ||
                   cpf->getHeader().basicInfo2Header()->target_class == CPFHeader::TargetClassEnum::ASYNC_TRANSPONDER ||
                   !cpf->getHeader().basicInfo2Header()->tiv_compatible || duration.count() <= 0)
                {
                    error_list.append({ErrorEnum::CPF_INVALID, filename + " is invalid."});
                }
                else if(p_end<start)
                {
                    error_list.append({ErrorEnum::CPF_OLD, filename + " is old."});
                }
                else
                {
                    list_cpf.append(cpf);
                }
            }
        }
        else
        {
            error_list.append({ErrorEnum::CPF_LOAD_FAILED, filename + " load failed."});
        }
    }

    if(list_cpf.isEmpty())
    {
        error_list.append({ErrorEnum::CPF_NOT_FOUND,
                           "CPF not found for space object with norad +'"+object.getNorad()+"'."});
        return SalaraInformation(error_list);
    }

    // TODO: this was an operator, but the comparation is rather strange. It should not be an opperator, but it should
    // be moved elsewhere.
    auto compCPF = [](const CPF& a, const CPF& b) -> bool
    {
        if (!a.getHeader().basicInfo1Header() || !b.getHeader().basicInfo1Header() ||
                !a.getHeader().basicInfo2Header() || !b.getHeader().basicInfo2Header())
            return false;

        // Son satelites diferentes.
        if(a.getHeader().basicInfo2Header()->norad != b.getHeader().basicInfo2Header()->norad)
            return a.getHeader().basicInfo2Header()->norad > b.getHeader().basicInfo2Header()->norad;
        // Son iguales con fechas diferentes.
        else if(a.getHeader().basicInfo1Header()->cpf_production_date !=
                b.getHeader().basicInfo1Header()->cpf_production_date)

            return a.getHeader().basicInfo1Header()->cpf_production_date >
                    b.getHeader().basicInfo1Header()->cpf_production_date;

        // Son iguales con fechas iguales y distinto proveedor.
        else if(a.getHeader().basicInfo1Header()->cpf_source != b.getHeader().basicInfo1Header()->cpf_source)
            return a.getHeader().basicInfo1Header()->cpf_source > b.getHeader().basicInfo1Header()->cpf_source;

        // Son iguales con fechas iguales y mismo proveedor. Vemos secuencia.
        else if(a.getHeader().basicInfo1Header()->cpf_sequence_number !=
                b.getHeader().basicInfo1Header()->cpf_sequence_number)

            return a.getHeader().basicInfo1Header()->cpf_sequence_number >
                    b.getHeader().basicInfo1Header()->cpf_sequence_number;
        else
            return false;
    };

    // Now sort the cpf list.
    std::sort(list_cpf.begin(), list_cpf.end(), [&tle_prior, compCPF](const auto& a, const auto& b)
    {
        if (!a->getHeader().basicInfo1Header() || !b->getHeader().basicInfo1Header())
            return false;

        if(tle_prior == PriorityTLE::LOWEST_PRIORITY)
        {
            QString source_a = QString::fromStdString(a->getHeader().basicInfo1Header()->cpf_source).toLower();
            QString source_b = QString::fromStdString(b->getHeader().basicInfo1Header()->cpf_source).toLower();

            if(source_a == "tle" && source_b == "tle")
                return compCPF(*a,*b);
            else if (source_a == "tle" && source_b != "tle")
                return false;
            else if (source_a != "tle" && source_b == "tle")
               return true;
        }
        return compCPF(*a,*b);
    });

    // For this option we select the most current CPF.
    if(selection_option == CPFFileManager::SelectionOption::MOST_CURRENT         ||
       selection_option == CPFFileManager::SelectionOption::MOST_CURRENT_REDUCE  ||
       selection_option == CPFFileManager::SelectionOption::MOST_CURRENT_FIXED)
    {

        auto it = list_cpf.cbegin();
        bool found = false;
        while (!found && it != list_cpf.cend())
        {
            // Prediction end and start time.
            QDateTime p_end = GlobalUtils::timePointToQDateTime((*it)->getHeader().basicInfo2Header()->end_time);
            QDateTime p_start = GlobalUtils::timePointToQDateTime((*it)->getHeader().basicInfo2Header()->start_time);

            if(!found && selection_option == CPFFileManager::SelectionOption::MOST_CURRENT_FIXED)
            {
                if(end.secsTo(p_end) > 0)
                {
                    t_days = p_start.secsTo(p_end)/86400.0;
                    r_days = start.secsTo(p_end)/86400.0;
                    c_days = start.secsTo(end)/86400.0;
                    found = true;
                    cpf_final = *it;
                }
            }
            else if(!found)
            {
                t_days = p_start.secsTo(p_end)/86400.0;
                r_days = start.secsTo(p_end)/86400.0;
                c_days = r_days;
                found = true;
                cpf_final = *it;
            }

            it++;
        }
    }
    // Para esta opción buscamos el CPF que contenga mayor número de días (comenzando siempre por el más nuevo).
    else if(selection_option == CPFFileManager::SelectionOption::MAXIMIZE_DAYS ||
            selection_option == CPFFileManager::SelectionOption::MAXIMIZE_DAYS_REDUCE)
    {
        int max_sec = 0;
        for(auto&& cpf : list_cpf)
        {
            // Prediction end and start time.
            QDateTime p_end = GlobalUtils::timePointToQDateTime(cpf->getHeader().basicInfo2Header()->end_time);
            QDateTime p_start = GlobalUtils::timePointToQDateTime(cpf->getHeader().basicInfo2Header()->start_time);

            // Have data and have more duration.
            if(start.secsTo(p_end)>max_sec)
            {
                t_days = p_start.secsTo(p_end)/86400.0;
                r_days = start.secsTo(p_end)/86400.0;
                c_days = r_days;
                max_sec = static_cast<int>(start.secsTo(p_end));
                cpf_final = cpf;
            }
        }
    }

    if(!cpf_final)
    {
        error_list.append({ErrorEnum::CPF_NOT_FOUND,
                           "CPF not found for space object with norad +'"+object.getNorad()+"'."});
    }

    return SalaraInformation(error_list);
}


QString CPFFileManager::findCPF(const QString &cpf_name, int days_after)
{
    QString current_cpf_path = SalaraSettings::instance().getGlobalConfigString(
                "SalaraProjectDataPaths/SP_CurrentCPF");
    QString current_path = current_cpf_path + '/' + cpf_name;

    if (QFile::exists(current_path))
        return current_path;

    QStringList splitted = cpf_name.split('_');
    if (splitted.size() == 4)
    {
        QDate date = QDate::fromString(splitted[2], "yyMMdd");
        date = date.addYears(100);
        // TODO: search days configurable
        QString hist_cpf_path = SalaraSettings::instance().getGlobalConfigString(
                    "SalaraProjectDataPaths/SP_HistoricalCPF");
        for (int i = 0; i < days_after; i++)
        {
            QString day_path = hist_cpf_path + '/' + date.toString("yyyyMMdd") + '/' + cpf_name;
            if (QFile::exists(day_path))
                return day_path;
            date = date.addDays(1);
        }
    }

    return {};
}
