#ifndef SRC_HTTP_HTTPRESPONSE_H
#define SRC_HTTP_HTTPRESPONSE_H

#include <unordered_map>
#include <sys/stat.h>
#include <assert.h>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>

#include "Buffer.h"
#include "Log.h"

#define HTTP_RESPONSE_MAX_LINE 256

class HttpResponse {
public:
    enum HTTP_RESPONSE_CODE
    {
        UNKNOW = -1,
        OK = 200,
        BAD_REQUEST = 400,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
    };
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir, std::string& path, bool isKeepAlive = false, int code = -1);
    void makeResponse(Buffer& buff);
    void unmapFile();
    char* getFile() { return m_mmFile; };
    size_t getFileLen() const { return m_mmFileStat.st_size; };
    void errorContent(Buffer& buff, std::string message);
    int getCode() const { return m_code; }

private:
    void addStateLine(Buffer &buff);
    void addHeader(Buffer &buff);
    void addContent(Buffer &buff);

    void sendErrorHtml();
    std::string getFileType();

    int m_code;
    bool m_isKeepAlive;

    std::string m_path;
    std::string m_srcDir;
    
    char* m_mmFile; 
    struct stat m_mmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> ERROR_CODE_PATH;
};


#endif //HTTP_RESPONSE_H