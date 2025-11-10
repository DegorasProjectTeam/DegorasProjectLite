#pragma once

#include <QList>
#include <QStringList>
#include <QString>
#include <QJsonObject>
#include <QJsonValue>
#include <QDateTime>

#include "class_prediction.h"
#include "class_predictionmodel.h"
#include "class_salarainformation.h"
#include "class_salarainformation.h"
#include "spcore_global.h"

class SP_CORE_EXPORT PredictionFileManager
{
public:

    enum ErrorEnum
    {
        DATAFILE_NOT_OPEN,
        SCHEMEFILE_NOT_OPEN,
        DATAFILE_INVALID,
        SCHEMEFILE_INVALID,
        INTEGRITY_ERROR
    };

    // TODO: maybe this class, should be as spaceobjectsmodelloader. There should be an instance watching the file or db
    // of predictions and that can offer list of predictions or predictions model.
    static const QMap<ErrorEnum, QString> ErrorEnumStringMap;

    // Static class. Delete constructor.
    PredictionFileManager() = delete;

    // Load methods.
    static SalaraInformation loadPredictionsDataAsModel(QString path_data, QString path_scheme,
                                          PredictionModel* model, QDateTime& versiondate);

    // Save methods.
    static SalaraInformation savePredictionsData(QString path_data, const QDateTime& creationdate, const QJsonArray& list);

    // Find methods.
    static SalaraInformation findPrediction(const QDateTime &pass_start, const QString& norad,
                                            std::shared_ptr<Prediction> &prediction,
                                            const QString &pred_path = "");
};
