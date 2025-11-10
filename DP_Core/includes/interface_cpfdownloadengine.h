#pragma once

#include "interface_plugin.h"
#include "class_salarainformation.h"

class SP_CORE_EXPORT CPFDownloadEngine : public SPPlugin
{
    Q_OBJECT
public:

    enum ErrorEnum
    {
        NOT_ERROR,
        LISTING_FAILED,
        DOWNLOAD_FAILED,
        ENGINE_BUSY,
        PERMISSIONS_ERROR
    };

    enum ClassificationEnum
    {
        NORMAL_ENGINE,
        CAMPAIGN_ENGINE
    };

    inline static const QMap<ErrorEnum, QString> ErrorEnumStringMap
    {
        {CPFDownloadEngine::ErrorEnum::LISTING_FAILED,
         "Listing failed. Impossible to connect to retrieve folder contents."},
        {CPFDownloadEngine::ErrorEnum::DOWNLOAD_FAILED,
         "Download failed. Impossible to connect to download the file."},
        {CPFDownloadEngine::ErrorEnum::ENGINE_BUSY,
         "Engine is busy. Impossible to accept this request."},
        {CPFDownloadEngine::ErrorEnum::PERMISSIONS_ERROR,
         "Denied permits. Impossible to create a temporary directory."}
    };

    CPFDownloadEngine(ClassificationEnum clas) :
        SPPlugin(PluginCategory::CPF_DOWNLOAD_ENGINE), classification(clas){}
    virtual ~CPFDownloadEngine() = default;

    virtual SalaraInformation listCPFs(QStringList& files) = 0;
    virtual SalaraInformation getCPF(const QString &file_name, const QString& dest_path) = 0;

    inline ClassificationEnum getEngineClassification() const {return this->classification;}

signals:
    void currentDownloadProgressState(int now, int total);

private:
    ClassificationEnum classification;
};

QT_BEGIN_NAMESPACE
#define CPFDownloadEngine_iid "SALARA_PROJECT.Interface_CPFDownloadEngine"
Q_DECLARE_INTERFACE(CPFDownloadEngine, CPFDownloadEngine_iid)
QT_END_NAMESPACE
Q_DECLARE_METATYPE(CPFDownloadEngine*)
