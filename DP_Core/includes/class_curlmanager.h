#pragma once

#include "spcore_global.h"

#include <QObject>
#include <QFutureWatcher>

#include <atomic>
#include <mutex>

typedef void CURL;

class SP_CORE_EXPORT CurlManager : public QObject
{
    Q_OBJECT
public:

    enum ResultCodes
    {
        CODE_OK = 0,
        CODE_UNSUPPORTED_PROTOCOL,    /* 1 */
        CODE_FAILED_INIT,             /* 2 */
        CODE_URL_MALFORMAT,           /* 3 */
        CODE_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
                                          7.17.0, reused in April 2011 for 7.21.5] */
        CODE_COULDNT_RESOLVE_PROXY,   /* 5 */
        CODE_COULDNT_RESOLVE_HOST,    /* 6 */
        CODE_COULDNT_CONNECT,         /* 7 */
        CODE_WEIRD_SERVER_REPLY,      /* 8 */
        CODE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                          due to lack of access - when login fails
                                          this is not returned. */
        CODE_FTP_ACCEPT_FAILED,       /* 10 - [was obsoleted in April 2006 for
                                          7.15.4, reused in Dec 2011 for 7.24.0]*/
        CODE_FTP_WEIRD_PASS_REPLY,    /* 11 */
        CODE_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server
                                          [was obsoleted in August 2007 for 7.17.0,
                                          reused in Dec 2011 for 7.24.0]*/
        CODE_FTP_WEIRD_PASV_REPLY,    /* 13 */
        CODE_FTP_WEIRD_227_FORMAT,    /* 14 */
        CODE_FTP_CANT_GET_HOST,       /* 15 */
        CODE_HTTP2,                   /* 16 - A problem in the http2 framing layer.
                                          [was obsoleted in August 2007 for 7.17.0,
                                          reused in July 2014 for 7.38.0] */
        CODE_FTP_COULDNT_SET_TYPE,    /* 17 */
        CODE_PARTIAL_FILE,            /* 18 */
        CODE_FTP_COULDNT_RETR_FILE,   /* 19 */
        CODE_OBSOLETE20,              /* 20 - NOT USED */
        CODE_QUOTE_ERROR,             /* 21 - quote command failure */
        CODE_HTTP_RETURNED_ERROR,     /* 22 */
        CODE_WRITE_ERROR,             /* 23 */
        CODE_OBSOLETE24,              /* 24 - NOT USED */
        CODE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
        CODE_READ_ERROR,              /* 26 - couldn't open/read from file */
        CODE_OUT_OF_MEMORY,           /* 27 */
        /* Note: CODE_OUT_OF_MEMORY may sometimes indicate a conversion error
                 instead of a memory allocation error if CURL_DOES_CONVERSIONS
                 is defined
        */
        CODE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
        CODE_OBSOLETE29,              /* 29 - NOT USED */
        CODE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
        CODE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
        CODE_OBSOLETE32,              /* 32 - NOT USED */
        CODE_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
        CODE_HTTP_POST_ERROR,         /* 34 */
        CODE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
        CODE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
        CODE_FILE_COULDNT_READ_FILE,  /* 37 */
        CODE_LDAP_CANNOT_BIND,        /* 38 */
        CODE_LDAP_SEARCH_FAILED,      /* 39 */
        CODE_OBSOLETE40,              /* 40 - NOT USED */
        CODE_FUNCTION_NOT_FOUND,      /* 41 - NOT USED starting with 7.53.0 */
        CODE_ABORTED_BY_CALLBACK,     /* 42 */
        CODE_BAD_FUNCTION_ARGUMENT,   /* 43 */
        CODE_OBSOLETE44,              /* 44 - NOT USED */
        CODE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
        CODE_OBSOLETE46,              /* 46 - NOT USED */
        CODE_TOO_MANY_REDIRECTS,      /* 47 - catch endless re-direct loops */
        CODE_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
        CODE_TELNET_OPTION_SYNTAX,    /* 49 - Malformed telnet option */
        CODE_OBSOLETE50,              /* 50 - NOT USED */
        CODE_OBSOLETE51,              /* 51 - NOT USED */
        CODE_GOT_NOTHING,             /* 52 - when this is a specific error */
        CODE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
        CODE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                          default */
        CODE_SEND_ERROR,              /* 55 - failed sending network data */
        CODE_RECV_ERROR,              /* 56 - failure in receiving network data */
        CODE_OBSOLETE57,              /* 57 - NOT IN USE */
        CODE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
        CODE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
        CODE_PEER_FAILED_VERIFICATION, /* 60 - peer's certificate or fingerprint
                                           wasn't verified fine */
        CODE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
        CODE_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
        CODE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
        CODE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
        CODE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
                                          that failed */
        CODE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
        CODE_LOGIN_DENIED,            /* 67 - user, password or similar was not
                                          accepted and we failed to login */
        CODE_TFTP_NOTFOUND,           /* 68 - file not found on server */
        CODE_TFTP_PERM,               /* 69 - permission problem on server */
        CODE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
        CODE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
        CODE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
        CODE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
        CODE_TFTP_NOSUCHUSER,         /* 74 - No such user */
        CODE_CONV_FAILED,             /* 75 - conversion failed */
        CODE_CONV_REQD,               /* 76 - caller must register conversion
                                          callbacks using curl_easy_setopt options
                                          CURLOPT_CONV_FROM_NETWORK_FUNCTION,
                                          CURLOPT_CONV_TO_NETWORK_FUNCTION, and
                                          CURLOPT_CONV_FROM_UTF8_FUNCTION */
        CODE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
                                          or wrong format */
        CODE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
        CODE_SSH,                     /* 79 - error from the SSH layer, somewhat
                                          generic so the error message will be of
                                          interest when this has happened */

        CODE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
                                          connection */
        CODE_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                          wait till it's ready and try again (Added
                                          in 7.18.2) */
        CODE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
                                          wrong format (Added in 7.19.0) */
        CODE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
                                          7.19.0) */
        CODE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
        CODE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
        CODE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
        CODE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
        CODE_CHUNK_FAILED,            /* 88 - chunk callback reported error */
        CODE_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the
                                          session will be queued */
        CODE_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not
                                           match */
        CODE_SSL_INVALIDCERTSTATUS,   /* 91 - invalid certificate status */
        CODE_HTTP2_STREAM,            /* 92 - stream error in HTTP/2 framing layer
                                          */
        CODE_RECURSIVE_API_CALL,      /* 93 - an api function was called from
                                          inside a callback */
        CODE_END
    };

    bool isWorking() const;
    ResultCodes getLastResultCode() const;
    QString getLastResultString() const;

    void setSSLVerification(bool);
    void setVerbose(bool);
    void setTimeout(int time_sec);
    void setProxy(const QString& url, int port, bool tunnel = true);
    void setFollowRedirection(bool follow);

    bool listFolders(const QString& url, int port, QStringList& buffer);
    bool downloadFile(const QString &url_path, int port, const QString &dest_file_path);
    bool downloadToString(const QString &url_path, int port, QString &result_string);
    bool uploadFile(const QString &filepath, const QString &url_path, int port);

    bool listFoldersBlocking(const QString &url, int port, QStringList &buffer);
    bool downloadFileBlocking(const QString &url_path, int port, const QString &dest_file_path);
    bool downloadToStringBlocking(const QString &url_path, int port, QString &result_string);
    bool uploadFileBlocking(const QString &filepath, const QString &url_path, int port);

signals:
    void currentProgressStateChanged(int remaining, int total);
    void jobFinished();

private:
    friend class CurlManagerFactory;
    explicit CurlManager(QObject *parent = nullptr);

    ~CurlManager() override;

    Q_DISABLE_COPY_MOVE(CurlManager)

    using QObject::setParent;

    static size_t writeDirectoryCallback(char *contents, size_t size, size_t nmemb, QStringList *buffer);
    static int progressCallback(CurlManager* ptr, long long dltotal, long long dlnow);
    static size_t writeFileCallback(void *ptr, size_t size, size_t nmemb, FILE *stream);
    static size_t writeStringCallback(char *contents, size_t size, size_t nmemb, QString *buffer);

    QFutureWatcher<ResultCodes> future_watcher;
    CURL *curl;
    ResultCodes last_result;
    QString last_result_string;

    mutable std::mutex mutex;
    std::atomic_bool working;

};

