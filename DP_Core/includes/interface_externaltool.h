#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QVariant>

#include "interface_plugin.h"
#include "spcore_global.h"

class SP_CORE_EXPORT ExternalTool : public SPPlugin
{
    Q_OBJECT
public:

    enum ErrorEnum
    {
        NOT_ERROR,
        ConfigFileNotFound,
        WrongInputData,
        ErrorWhileProcesingData
    };

    struct ExternalToolError
    {
        ExternalToolError(const int& n, const QString s): error_number(n),error_string(s){}
        ExternalToolError():error_number(ErrorEnum::NOT_ERROR),error_string(""){}
        int error_number;
        QString error_string;
    };

    ExternalTool():SPPlugin(PluginCategory::EXTERNAL_TOOL){}

    virtual ~ExternalTool() = default;

    // Virtual methods.
    virtual ExternalToolError execExternalTool() = 0;
};

QT_BEGIN_NAMESPACE
#define ExternalTool_iid "SALARA_PROJECT.Interface_ExternalTool"
Q_DECLARE_INTERFACE(ExternalTool, ExternalTool_iid)
QT_END_NAMESPACE
Q_DECLARE_METATYPE(ExternalTool*)
