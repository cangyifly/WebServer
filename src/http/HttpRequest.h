#ifndef SRC_HTTP_HTTPREQUEST_H
#define SRC_HTTP_HTTPREQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <regex>
#include <mysql/mysql.h>

#include "Buffer.h"
#include "Log.h"
#include "SqlConnRAII.h"
#include "SqlConnPool.h"

#define HTTP_REQUEST_HEADER_CONNECTION "Connection"
#define HTTP_REQUEST_HEADER_KEEP_ALIVE "keep-alive"

#define HTTP_REQUEST_HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_REQUEST_HEADER_CONTENT_TYPE_DEFAULT "application/x-www-form-urlencoded"

#define HTTP_METHOD_POST "POST"

class HttpRequest 
{
public:
    enum PARSE_STATE 
    {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,        
    };

    enum HTTP_CODE 
    {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    
    HttpRequest() { init(); }
    ~HttpRequest() = default;

    void init();
    bool parse(Buffer& buff);

    std::string path() const { return m_path; };
    std::string& path() { return m_path; };
    std::string method() const { return m_method; };
    std::string version() const { return m_version; };
    std::string getPost(const std::string& key) const;
    std::string getPost(const char* key) const;

    bool isKeepAlive() const;

    /* 
    todo 
    void HttpConn::ParseFormData() {}
    void HttpConn::ParseJson() {}
    */

private:
    bool parseRequestLine(const std::string& line);
    void parseHeader(const std::string& line);
    void parseBody(const std::string& line);

    void parsePath();
    void parsePost();
    void parseFromUrlencoded();

    static bool userVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE m_parseState;
    std::string m_method;
    std::string m_path;
    std::string m_version;
    std::string m_body;
    std::unordered_map<std::string, std::string> m_header;
    std::unordered_map<std::string, std::string> m_post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    static int ConvertToHex(char ch);
};


#endif //SRC_HTTP_HTTPREQUEST_H