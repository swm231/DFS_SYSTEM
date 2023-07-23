#pragma once

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>

class SqlConnPool{
public:
    SqlConnPool();
    ~SqlConnPool();

    MYSQL *GetConn();
    void FreeConn(MYSQL *conn);

    void Init(const char *host, const char *user, 
        const char *pwd, const char *dbName, int connSize = 16);
    void ClosePool();

private:
    std::queue<MYSQL*> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};