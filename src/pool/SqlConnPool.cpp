#include "SqlConnPool.h"

SqlConnPool::SqlConnPool(){};

SqlConnPool::~SqlConnPool()
{
    close();
    sem_destroy(&m_semId);
}

SqlConnPool* SqlConnPool::GetInstance()
{
    static SqlConnPool instance;
    return &instance;
}

void SqlConnPool::init(const char* host, int port, const char* user, const char* pwd, const char* dbName, int connSize)
{
    assert(connSize >0);
    // 创建mysql连接队列
    for (int i = 0; i < connSize; i++)
    {
        MYSQL *mysql = nullptr;
        mysql = mysql_init(mysql);

        if (!mysql)
        {
            LOG_ERROR("Mysql %d init failed!", i);
            assert(mysql);
        }

        mysql = mysql_real_connect(mysql, host, user, pwd, dbName, port, nullptr, 0);
        if (!mysql)
        {
            LOG_ERROR("Mysql %d connect failed!", i);
        }
        m_connQueue.push(mysql);
    }
    m_max_count = connSize;
    sem_init(&m_semId, 0, connSize);
}

MYSQL* SqlConnPool::GetConn()
{
    MYSQL *mysql = nullptr;
    if (m_connQueue.empty())
    {
        LOG_WARN("SqlConnPool busy!");
        return mysql;
    }

    sem_wait(&m_semId);
    {
        std::lock_guard<std::mutex> loker(m_mutex);
        mysql = m_connQueue.front();
        m_connQueue.pop();
    }
    return mysql;
}

void SqlConnPool::FreeConn(MYSQL* mysql)
{
    assert(mysql);
    std::lock_guard<std::mutex> loker(m_mutex);
    m_connQueue.push(mysql);
    sem_post(&m_semId);
}

void SqlConnPool::close()
{
    std::lock_guard<std::mutex> loker(m_mutex);
    while (!m_connQueue.empty())
    {
        MYSQL *mysql = m_connQueue.front();
        m_connQueue.pop();

        mysql_close(mysql);
    }
    mysql_library_end();
}

int SqlConnPool::GetFreeConnCount()
{
    std::lock_guard<std::mutex> loker(m_mutex);
    return m_connQueue.size();
}