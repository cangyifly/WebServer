#include "Log.h"

using namespace std;

Log* Log::GetInstance()
{
    static Log instance;
    return &instance;
}

Log::Log()
{
    m_lineCount = 0;
    // 默认不开启异步写线程
    m_isAsync = false;
    m_isOpen = false;
    m_writeThread = nullptr;
    m_deque = nullptr;
    m_toDay = 0;
    m_fp = nullptr;
    m_maxLines = MAX_LINES;
}

Log::~Log()
{
    // 如果异步线程存在
    if (m_writeThread && m_writeThread->joinable())
    {
        while (!m_deque->empty())
        {
            m_deque->flush();
        }
        m_deque->close();
        m_writeThread->join();
    }

    if (m_fp)
    {
        std::lock_guard<mutex> loker(m_mutex);
        flush();
        fclose(m_fp);
    }
}

int Log::getLevel()
{
    std::lock_guard<mutex> loker(m_mutex);
    return m_level;
}

void Log::setLevel(int level)
{
    std::lock_guard<mutex> loker(m_mutex);
    m_level = level;
}

void Log::init(int level, const char *path, const char *suffix, int maxQueueCapacity)
{
    m_isOpen = true;
    m_level = level;
    if (maxQueueCapacity > 0)
    {
        // 开启异步写线程
        m_isAsync = true;
        // 创建队列
        if (!m_deque)
        {
            std::unique_ptr<BlockQueue<std::string>> newQueue(new BlockQueue<std::string>);
            m_deque = move(newQueue);

            // 创建线程
            std::unique_ptr<std::thread> newThread(new thread(FlushLogThread));
            m_writeThread = move(newThread);
        }
    }
    else
    {
        m_isAsync = false;
    }

    m_lineCount = 0;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;

    // 日志目录路径
    m_path = path;
    // 日志文件后缀
    m_suffix = suffix;

    // 日志文件
    char filename[LOG_NAME_LEN] = {0};
    snprintf(filename, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", m_path, t.tm_year + 1900, t.tm_mon + 1,
            t.tm_mday, m_suffix);
    
    m_toDay = t.tm_mday;

    {
        std::lock_guard<mutex> locker(m_mutex);
        // 清空缓冲区
        m_buffer.clear();
        // 关闭m_fp
        if (m_fp)
        {
            flush();
            fclose(m_fp);
        }

        m_fp = fopen(filename, "a");
        if (m_fp == nullptr)
        {
            // 创建文件夹，再重新创建文件
            mkdir(m_path, 0777);
            m_fp = fopen(filename, "a");
        }

        assert(m_fp != nullptr);
    }
}

void Log::write(int level, const char* format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);

    time_t tSec = now.tv_sec;

    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;

    // 如果文件日期不一致或者文件已经写满了
    if (m_toDay != t.tm_mday || (m_lineCount && (m_lineCount % m_maxLines == 0)))
    {
        std::unique_lock<mutex> locker(m_mutex);
        locker.unlock();

        char newFilename[LOG_NAME_LEN] = {0};
        char tail[36] = {0};

        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        // 如果当前文件日期不一致
        if (m_toDay != t.tm_mday)
        {
            snprintf(newFilename, LOG_NAME_LEN - 72, "%s/%s%s", m_path, tail, m_suffix);
            m_toDay = t.tm_mday;
            m_lineCount = 0;
        }
        else
        {
            snprintf(newFilename, LOG_NAME_LEN - 72, "%s/%s-%d%s", m_path, tail, (m_lineCount / m_maxLines) , m_suffix);
        }

        locker.lock();
        flush();
        fclose(m_fp);
        m_fp = fopen(newFilename, "a");
        assert(m_fp != nullptr);
    }

    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_lineCount++;
        int n = snprintf(m_buffer.writePtr(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        m_buffer.hasWritten(n);
        appendLogLevelTitle(level);

        // 获取变长参数列表
        va_list vaList;
        va_start(vaList, format);
        int m = vsnprintf(m_buffer.writePtr(), m_buffer.writableBytes(), format, vaList);
        va_end(vaList);
        m_buffer.hasWritten(m);
        m_buffer.append("\n\0", 2);

        // 如果开启了异步写线程，则暂存到阻塞队列上
        if (m_isAsync && m_deque && !m_deque->full())
        {
            m_deque->push_back(m_buffer.retriveToStr());
        }
        // 否则直接写入文件
        else
        {
            // fputs将内容写入缓冲区
            fputs(m_buffer.readPtrConst(), m_fp);
            // fwrite(m_buffer.readPtr(), sizeof(char), m_buffer.readableBytes(), m_fp); 
        }
        m_buffer.clear();  
    }
    
}

void Log::appendLogLevelTitle(int level)
{
    int len = 9;
    switch (level)
    {
    case LOG_LEVEL_DEBUG:
        m_buffer.append("[debug]: ", len);
        break;

    case LOG_LEVEL_INFO:
        m_buffer.append("[info] : ", len);
        break;

    case LOG_LEVEL_WARN:
        m_buffer.append("[warn] : ", len);
        break;

    case LOG_LEVEL_ERROR:
        m_buffer.append("[error]: ", len);
        break;
    
    default:
        m_buffer.append("[info] : ", len);
        break;
    }
}

void Log::flush()
{
    if (m_isAsync)
    {
        m_deque->flush();
    }
    // 强制将缓冲区的内容写入文件
    fflush(m_fp);
}

void Log::asyncWrite()
{
    string str = "";
    // 不断将队列中的内容写入缓存区
    while (m_deque->pop(str))
    {
        std::lock_guard<mutex> locker(m_mutex);
        fputs(str.c_str(), m_fp);
    }
}

void Log::FlushLogThread()
{
    Log::GetInstance()->asyncWrite();
}
