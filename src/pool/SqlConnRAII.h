#ifndef SRC_POOL_SQLCONNRAII_H
#define SRC_POOL_SQLCONNRAII_H
#include "SqlConnPool.h"

class SqlConnRAII
{
public:
    SqlConnRAII(MYSQL** mysql, SqlConnPool* sqlConnPool)
    {
        assert(sqlConnPool);
        *mysql = sqlConnPool->GetConn();
        m_mysql = *mysql;
        m_sqlConnPool = sqlConnPool;
    }

    ~SqlConnRAII()
    {
        if (m_mysql)
        {
            m_sqlConnPool->FreeConn(m_mysql);
        }
    }

private:
    MYSQL *m_mysql;
    SqlConnPool* m_sqlConnPool;
};
#endif