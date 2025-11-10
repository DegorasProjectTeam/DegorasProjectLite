#include "includes/class_curlmanagerfactory.h"
#include "includes/class_curlmanager.h"

#include <curl/curl.h>

CurlManager* CurlManagerFactory::createManager()
{
    auto ptr = new CurlManager;
    this->curl_managers_.push_back(ptr);
    return ptr;
}

bool CurlManagerFactory::destroyManager(CurlManager *manager)
{
    bool result = false;
    auto it = std::find(this->curl_managers_.cbegin(), this->curl_managers_.cend(), manager);
    if (it != this->curl_managers_.cend())
    {
        this->curl_managers_.erase(it);
        result = true;
    }
    return result;
}

CurlManagerFactory &CurlManagerFactory::instance()
{
    static CurlManagerFactory instance;
    return instance;
}

CurlManagerFactory::CurlManagerFactory()
{
    // libCUrl initialization
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CurlManagerFactory::~CurlManagerFactory()
{
    for (const auto& ptr : this->curl_managers_)
        delete ptr;
    this->curl_managers_.clear();
    curl_global_cleanup();
}
