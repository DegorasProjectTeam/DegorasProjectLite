#include "includes/global_texts.h"
#include "includes/class_predictionfilemanager.h"
#include "includes/global_texts.h"
#include "includes/class_salarainformation.h"
#include "includes/class_salarasettings.h"
#include "includes/class_spaceobjectsmodelloader.h"

#include <QJsonDocument>
#include <QDebug>
#include <QFile>


const QMap<PredictionFileManager::ErrorEnum, QString> PredictionFileManager::ErrorEnumStringMap =
{
    {PredictionFileManager::ErrorEnum::DATAFILE_INVALID,
     "The json file '"+QString(FILE_PREDICTIONSDATA)+"' is invalid."},
    {PredictionFileManager::ErrorEnum::SCHEMEFILE_INVALID,
     "The json file '"+QString(FILE_PREDICTIONSSCHEME)+"' is invalid."},
    {PredictionFileManager::ErrorEnum::DATAFILE_NOT_OPEN,
     "The json file '"+QString(FILE_PREDICTIONSDATA)+"' cannot be open."},
    {PredictionFileManager::ErrorEnum::SCHEMEFILE_NOT_OPEN,
     "The json file '"+QString(FILE_PREDICTIONSSCHEME)+"' cannot be open."},
    {PredictionFileManager::ErrorEnum::INTEGRITY_ERROR,
     "The json file '"+QString(FILE_PREDICTIONSDATA)+"' has integrity errors."}
};


SalaraInformation PredictionFileManager::loadPredictionsDataAsModel(QString path_data, QString path_scheme,
                                                     PredictionModel* model, QDateTime& creationdate)
{
    // Variables.
    SalaraInformation::ErrorList error_list;
    QString predictionsscheme_string, predictionsfile_string;
    QJsonDocument predictionsscheme_jsondocument, predictionsfile_jsondocument;
    QJsonArray objectmasterscheme_jsonarray, predictionsfile_jsonarray;

    // Open and loads first the scheme.
    QFile predictionsscheme(path_scheme);
    if(!predictionsscheme.open(QIODevice::ReadOnly | QIODevice::Text))
            return SalaraInformation({PredictionFileManager::SCHEMEFILE_NOT_OPEN ,
                                      ErrorEnumStringMap[ErrorEnum::SCHEMEFILE_NOT_OPEN]}, path_scheme);
    predictionsscheme_string = predictionsscheme.readAll().simplified();
    predictionsscheme.close();
    predictionsscheme_jsondocument = QJsonDocument::fromJson(predictionsscheme_string.toUtf8());
    if (predictionsscheme_jsondocument.isNull())
        return SalaraInformation({PredictionFileManager::SCHEMEFILE_INVALID ,
                                              ErrorEnumStringMap[ErrorEnum::SCHEMEFILE_INVALID]}, path_scheme);
    objectmasterscheme_jsonarray = predictionsscheme_jsondocument["PredictionsSchemeArray"].toArray();
    model->setJsonScheme(objectmasterscheme_jsonarray);

    // Now load the data.
    QFile predictionsfile(path_data);
    if(!predictionsfile.open(QIODevice::ReadOnly | QIODevice::Text))
            return SalaraInformation({PredictionFileManager::DATAFILE_NOT_OPEN,
                                      ErrorEnumStringMap[ErrorEnum::DATAFILE_NOT_OPEN]}, path_data);
    predictionsfile_string = predictionsfile.readAll().simplified();
    predictionsfile.close();
    predictionsfile_jsondocument = QJsonDocument::fromJson(predictionsfile_string.toUtf8());
    if (predictionsfile_jsondocument.isNull())
        return SalaraInformation({PredictionFileManager::DATAFILE_INVALID,
                                  ErrorEnumStringMap[ErrorEnum::DATAFILE_INVALID]}, path_scheme);
    predictionsfile_jsonarray = predictionsfile_jsondocument["PredictionsArray"].toArray();

    // Create the model and set the data.
    PredictionModel::ErrorIndexList data_error = model->setJsonData(predictionsfile_jsonarray);

    // Save version and comment.
    QString aux = predictionsfile_jsondocument["CreationDateTime"].toString();
    creationdate = QDateTime::fromString(aux, DATETIME_STORESTRING);

    // Store data integrity errors in error list
    for (const auto& error : data_error)
    {
        error_list.append(qMakePair(ErrorEnum::INTEGRITY_ERROR,
                                    ErrorEnumStringMap[ErrorEnum::INTEGRITY_ERROR]+
                          " Row: " + QString::number(error.second.row() + 1) +
                          " - Column: " +  model->headerData(error.second.column(), Qt::Horizontal).toString() +
                          " -> " + PredictionModel::ErrorEnumStringMap[error.first]));
    }

    // Return the errors.
    return SalaraInformation(error_list);
}


SalaraInformation PredictionFileManager::savePredictionsData(QString path_data, const QDateTime& creationdate, const QJsonArray& list)
{
    // Creation date.
    QString creationstring = creationdate.toString(DATETIME_STORESTRING);

    // Variables.
    QJsonObject jsonobject;
    QFile predictionsdatafile(path_data);

    // Update the data.
    jsonobject.insert("CreationDateTime", creationstring);
    jsonobject.insert("PredictionsArray", list);

    // JsonDocument (it will save in the file).
    QJsonDocument jsondocument(jsonobject);

    // Save the data.
    if(!predictionsdatafile.open(QIODevice::WriteOnly))                   
        return SalaraInformation({DATAFILE_NOT_OPEN, ErrorEnumStringMap[ErrorEnum::DATAFILE_NOT_OPEN]});
    predictionsdatafile.write(jsondocument.toJson(QJsonDocument::Indented));
    predictionsdatafile.close();

    return SalaraInformation();
}

SalaraInformation PredictionFileManager::findPrediction(const QDateTime &pass_start, const QString &norad,
                                                        std::shared_ptr<Prediction> &prediction,
                                                        const QString &pred_path)
{
    SalaraInformation errors;
    QString dir_schemes = SalaraSettings::instance().getGlobalConfigString(
                "SalaraProjectSchemeFiles/SP_SchemeFiles");
    PredictionModel model;
    QString pred_path_selected = pred_path;

    if (pred_path_selected.isEmpty())
    {
        QString dir_currentprediction = SalaraSettings::instance().getGlobalConfigString(
                    "SalaraProjectDataPaths/SP_CurrentPredictions");
        pred_path_selected = dir_currentprediction + '/' + FILE_PREDICTIONSDATA;
    }

    QDateTime creation_date;
    errors.append(PredictionFileManager::loadPredictionsDataAsModel(pred_path_selected,
                                                                    dir_schemes + '/' + FILE_PREDICTIONSSCHEME,
                                                                    &model, creation_date));

    prediction.reset();


    if (model.rowCount() > 0)
    {
        int norad_col = model.findColumnSectionByIndex("Object_NORAD");
        int start_col = model.findColumnSectionByHeaderProperty("Title", "DateTimeArray");

        if (norad_col != -1 && start_col != -1)
        {
            int i = 0;
            QString norad_trimmed = norad.trimmed();
            bool found = false;
            while (i < model.rowCount() && !found)
            {
                QString obj_norad = model.data(model.index(i, norad_col)).toString();
                QDateTime pred_start = model.data(model.index(i, start_col), JsonTableModel::OBJECT_ROLE).toDateTime();

                if (norad_trimmed == obj_norad.trimmed() && pass_start == pred_start)
                    found = true;
                else
                    i++;
            }
            if (found)
            {
                auto object_it = SpaceObjectsModelLoader::instance().objectsMap().find(norad_trimmed);
                if (object_it != SpaceObjectsModelLoader::instance().objectsMap().end())
                    prediction.reset(model.getRowObject(i, *object_it));
                else
                    errors.append({{0, "Prediction found but related space object not found"}});
            }
            else
                errors.append({{0, "Prediction not found"}});
        }
        else
            errors.append({{0, "Cannot get norad or start time from predictions."}});
    }
    else
        errors.append({{0, "Predictions model is empty."}});

    return errors;

}


