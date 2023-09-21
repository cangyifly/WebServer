#include "WebServer.h"

#define SERVER_PORT 8888
#define ET_MODE 3
#define TIMEOUT_MS 60000

#define MYSQL_PORT 3306
#define MYSQL_CONNECT_USER "root"
#define MYSQL_CONNECT_PWD "root"
#define MYSQL_DATABSE_NAME "yourdb"
#define MYSQL_CONNECT_POLL_SIZE 12

#define THREAD_POOL_SIZE 6

#define LOG_IS_OPEN true
#define LOG_LEVEL LOG_LEVEL_DEBUG
#define LOG_BLOCKQUEUE_SIZE 1024
int main()
{
    WebServer server(SERVER_PORT, ET_MODE, TIMEOUT_MS, false,
                    MYSQL_PORT, MYSQL_CONNECT_USER, MYSQL_CONNECT_PWD, MYSQL_DATABSE_NAME, MYSQL_CONNECT_POLL_SIZE,
                    THREAD_POOL_SIZE, LOG_IS_OPEN, LOG_LEVEL, LOG_BLOCKQUEUE_SIZE);
    server.start();
    return 0;
}