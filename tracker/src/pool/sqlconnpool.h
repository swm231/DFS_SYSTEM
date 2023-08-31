#pragma once

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <semaphore.h>

#include "../log/log.h"

class SqlConnPool{
public:
    static SqlConnPool& Instance(){
        static SqlConnPool instance;
        return instance;
    }

    MYSQL *GetConn();
    void FreeConn(MYSQL *conn);

    void Init(const char *host, const char *user, 
        const char *pwd, const char *dbName, int connSize = 8);
    void ClosePool();

    void StartFresh();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;

    std::queue<MYSQL*> connQue_;
    std::mutex mtx_;
    sem_t semId_;
    bool stop_;

    std::unique_ptr<std::thread> FreshenThread_;
    void Freshen_();

};