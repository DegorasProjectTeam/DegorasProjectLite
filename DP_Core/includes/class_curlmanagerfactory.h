#pragma once

#include "spcore_global.h"

#include <QObject>

#include <memory>

class CurlManager;

// Factory creates each curlmanager and inits and destroy global library.
class SP_CORE_EXPORT CurlManagerFactory : public QObject
{
public:

    /**
     * @brief Creates a new CurlManager to access Curl functionality. There is no need release the pointer, since
     * all managers are eventually released. @see destroyManager
     * @return A pointer to CurlManager
     */
    CurlManager* createManager();
    /**
     * @brief Destroy a previously created CurlManager. There is no need to call this method, since are managers are
     * eventually destroyed. Nevertheless, you can use this method to release an unecessary manager.
     * @param manager, a smart pointer to
     */
    bool destroyManager(CurlManager* manager);

    // Instance.
    static CurlManagerFactory& instance();

private:
    CurlManagerFactory();
    ~CurlManagerFactory() override;

    Q_DISABLE_COPY_MOVE(CurlManagerFactory)

    std::list<CurlManager*> curl_managers_;
};


