#include "includes/class_curlmanager.h"
#include "includes/class_salarasettings.h"


#include <curl/curl.h>

#include <QtConcurrent/QtConcurrentRun>

#include <cstring>

// TODO: allow and/or manage redirections?

CurlManager::CurlManager(QObject *parent) : QObject(parent), working(false)
{
    this->curl = curl_easy_init();
    QObject::connect(&this->future_watcher, &QFutureWatcher<CurlManager::ResultCodes>::finished, this, [this]
    {
        last_result = this->future_watcher.result();
        working = false;
        emit this->jobFinished();
    });

    QString cacert_path = SalaraSettings::instance().getGlobalConfigString(
                "SalaraProjectConfigPaths/SP_ConfigFiles") + "/cacert.pem";

    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_CAINFO, cacert_path.toLatin1().data());
}

void CurlManager::setSSLVerification(bool enable)
{
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, enable ? 1L : 0L);
}

void CurlManager::setVerbose(bool enable)
{
    curl_easy_setopt(curl, CURLOPT_VERBOSE, enable ? 1L : 0L);
}

bool CurlManager::listFoldersBlocking(const QString &url, int port, QStringList &buffer)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;

        // Configure libCurl to list the folder in the next command
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1);
        curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_URL, url.toLatin1().data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDirectoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        this->last_result = static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));

        result = true;
        working = false;
        emit this->jobFinished();
    }
    return result;
}

bool CurlManager::listFolders(const QString &url, int port, QStringList &buffer)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;

        // Configure libCurl to list the folder in the next command
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 1);
        curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_URL, url.toLatin1().data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDirectoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        this->future_watcher.setFuture(QtConcurrent::run([this]
        {
            return static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));
        }));

        result = true;
    }
    return result;
}

bool CurlManager::downloadFileBlocking(const QString& url_path, int port, const QString& dest_file_path)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;
        FILE* dest_file = fopen(dest_file_path.toLatin1().data(),"wb");
        // Configure curl to download the opportunities file
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0);
        curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_URL, url_path.toLatin1().data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, dest_file);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);

        // Start the downloading operation
        this->last_result = static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));
        fclose(dest_file);
        result = true;
        working = false;
        emit this->jobFinished();
    }
    return result;
}


bool CurlManager::downloadFile(const QString& url_path, int port, const QString& dest_file_path)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;
        FILE* dest_file = fopen(dest_file_path.toLatin1().data(),"wb");
        // Configure curl to download the opportunities file
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0);
        curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_URL, url_path.toLatin1().data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, dest_file);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);

        // Start the downloading operation
        this->future_watcher.setFuture(QtConcurrent::run([this, dest_file]
        {
            CurlManager::ResultCodes res = static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));
            fclose(dest_file);
            return res;
        }));

        result = true;
    }
    return result;
}

bool CurlManager::downloadToStringBlocking(const QString& url_path, int port, QString& result_string)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;
        result_string.clear();
        // Configure curl to download the opportunities file
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0);
        curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_URL, url_path.toLatin1().data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStringCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result_string);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);

        this->last_result = static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));
        result = true;
        working = false;
        emit this->jobFinished();
    }
    return result;
}

bool CurlManager::uploadFileBlocking(const QString &filepath, const QString &url_path, int port)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;
        result = true;
        std::string path = filepath.toStdString();
        FILE *file_src = fopen(path.c_str(), "rb");

        if (file_src)
        {
            // Configure curl to download the opportunities file
            curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0);
            curl_easy_setopt(curl, CURLOPT_PORT, port);
            curl_easy_setopt(curl, CURLOPT_URL, url_path.toLatin1().data());
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_READDATA, file_src);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(QFile(filepath).size()));
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);

            this->last_result = static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));
            fclose(file_src);
        }
        else
        {
            this->last_result = CODE_FILE_COULDNT_READ_FILE;
        }
        working = false;
        emit this->jobFinished();
    }
    return result;
}

bool CurlManager::downloadToString(const QString& url_path, int port, QString& result_string)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;
        result_string.clear();
        // Configure curl to download the opportunities file
        curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0);
        curl_easy_setopt(curl, CURLOPT_PORT, port);
        curl_easy_setopt(curl, CURLOPT_URL, url_path.toLatin1().data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeStringCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result_string);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);

        // Start the downloading operation
        this->future_watcher.setFuture(QtConcurrent::run([this]
        {
            return static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));
        }));

        result = true;
    }
    return result;
}

bool CurlManager::uploadFile(const QString &filepath, const QString &url_path, int port)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    bool result = false;
    if (!working)
    {
        working = true;
        result = true;
        std::string path = filepath.toStdString();
        FILE *file_src = fopen(path.c_str(), "rb");

        if (file_src)
        {
            // Configure curl to download the opportunities file
            curl_easy_setopt(curl, CURLOPT_DIRLISTONLY, 0);
            curl_easy_setopt(curl, CURLOPT_PORT, port);
            curl_easy_setopt(curl, CURLOPT_URL, url_path.toLatin1().data());
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            curl_easy_setopt(curl, CURLOPT_READDATA, file_src);
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(QFile(filepath).size()));
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);

            // Start the downloading operation
            this->future_watcher.setFuture(QtConcurrent::run([this, file_src]
            {
                auto res = static_cast<CurlManager::ResultCodes>(curl_easy_perform(this->curl));
                fclose(file_src);
                return res;
            }));
        }
        else
        {
            this->last_result = CODE_FILE_COULDNT_READ_FILE;
        }

    }
    return result;
}


CurlManager::~CurlManager()
{
    // Finish libCUrl
    curl_easy_cleanup(curl);
}

bool CurlManager::isWorking() const
{
    return working;
}

CurlManager::ResultCodes CurlManager::getLastResultCode() const
{
    return working ? ResultCodes::CODE_END : last_result;
}

QString CurlManager::getLastResultString() const
{
    const char* error = curl_easy_strerror(static_cast<CURLcode>(last_result));

    if(working)
        return "";

    return QString(error);
}

void CurlManager::setTimeout(int time_sec)
{
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, time_sec);
}

void CurlManager::setProxy(const QString &url, int port, bool tunnel)
{
    QString url_with_port = url + ':' + QString::number(port);
    curl_easy_setopt(curl, CURLOPT_PROXY, url_with_port.toLatin1().data());
    curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, tunnel ? 1L : 0);
}

void CurlManager::setFollowRedirection(bool follow)
{
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, follow ? 1L : 0L);
}

size_t CurlManager::writeDirectoryCallback(char *contents, size_t size, size_t nmemb, QStringList *buffer)
{
    char* auxchar = new char[nmemb+1];
    std::strncpy(auxchar, contents, nmemb);
    auxchar[nmemb]='\0';

    QStringList splitter;
    size_t realsize = size * nmemb;
    QString aux = QString(auxchar);

    if(aux.contains('\r'))
        splitter = aux.split("\r\n");
    else if(aux.contains('\n'))
        splitter = aux.split('\n');
    else
        splitter.append(aux);
    splitter.removeAll("");
    splitter.removeAll(".");
    splitter.removeAll("..");
    buffer->append(splitter);

    delete []auxchar;
    return realsize;
}

size_t CurlManager::writeFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

size_t CurlManager::writeStringCallback(char *contents, size_t size, size_t nmemb, QString *buffer)
{
    char* auxchar = new char[size * nmemb + 1];
    std::strncpy(auxchar, contents, nmemb);
    auxchar[size * nmemb]='\0';

    buffer->append(auxchar);

    delete []auxchar;
    return size * nmemb;
}


int CurlManager::progressCallback(CurlManager *ptr, long long dltotal, long long dlnow)
{
    // Emit signal with current progress state
    if(dltotal!=0)
    {
        int total = static_cast<int>(dltotal);
        int remaining = static_cast<int>(dlnow);
        emit ptr->currentProgressStateChanged(remaining, total);
    }
    return 0;
}
