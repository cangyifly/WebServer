#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <thread>
#include <sys/stat.h>
#include <stdarg.h> // va_start

#include "BlockQueue.h"
#include "../buffer/Buffer.h"

#define LOG_LEVEL_DEFAULT 1
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_INFO  4


class Log
{
public:
    void init(int level = LOG_LEVEL_DEFAULT, const char *path = "./log", const char *suffix = ".log", int maxQueueCapacity = MAX_CAPACITY);

    static Log* GetInstance();
    static void FlushLogThread();

    void write(int level, const char *format, ...);
    void flush();

    int getLevel();
    void setLevel(int level);
    bool isOpen() { return m_isOpen; }
    
private:
    Log();
    void appendLogLevelTitle(int level);
    virtual ~Log();
    void asyncWrite();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* m_path;
    const char* m_suffix;

    int m_maxLines;
    int m_lineCount;
    int m_toDay;

    bool m_isOpen;

    Buffer m_buffer;
    int m_level;
    bool m_isAsync;

    FILE* m_fp;
    std::unique_ptr<BlockQueue<std::string>> m_deque;
    std::unique_ptr<std::thread> m_writeThread;
    std::mutex m_mutex;
};
// ## 可以将宏中的入参转为字符串
#define LOG_BASE(level, format, ...) \
    do \ 
    { \
        Log *log = Log::GetInstance(); \
        if (log->isOpen() && log->getLevel() <= level) \
        { \
            log->write(level, format, ##__VA__ARGS__); \
            log->flush; \
        } \
    } \
    while (0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(LOG_LEVEL_INFO, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(LOG_LEVEL_WARN, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)} while(0);

#endif