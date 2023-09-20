#ifndef SRC_POOL_SQLCONNPOOL_H
#define SRC_POOL_SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <assert.h>

#include "Log.h"

#define DEFAULT_CONN_SIZE 10
#define MYSQL_SQL_MAX_LEN 256

class SqlConnPool
{
public:
    static SqlConnPool* GetInstance();

    MYSQL *GetConn();
    void FreeConn(MYSQL* conn);
    int GetFreeConnCount();

    void init(const char* host, int port, const char* user, const char* pwd, const char* dbName, int connSize = DEFAULT_CONN_SIZE);
    void close();

private:
    SqlConnPool();
    ~SqlConnPool();

    int m_max_count;
    std::queue<MYSQL*> m_connQueue;
    std::mutex m_mutex;
    sem_t m_semId;
};

#endif