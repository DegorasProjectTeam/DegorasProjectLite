#pragma once

#include "interface_plugin.h"
#include "class_salarainformation.h"

class SP_CORE_EXPORT TLEDownloadEngine : public SPPlugin
{
    Q_OBJECT
public:

    enum ErrorEnum
    {
        NOT_ERROR,
        DOWNLOAD_FAILED,
        ENGINE_BUSY
    };

    TLEDownloadEngine() : SPPlugin(PluginCategory::TLE_DOWNLOAD_ENGINE){}
    virtual ~TLEDownloadEngine() = default;

    virtual SalaraInformation getTLE(const QString& norad, QString& result) = 0;

signals:
    void currentDownloadProgressState(int now, int total);

};

QT_BEGIN_NAMESPACE
#define TLEDownloadEngine_iid "SALARA_PROJECT.Interface_TLEDownloadEngine"
Q_DECLARE_INTERFACE(TLEDownloadEngine, TLEDownloadEngine_iid)
QT_END_NAMESPACE

Q_DECLARE_METATYPE(TLEDownloadEngine*)

