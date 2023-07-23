#include "sqlconnpool.h"

SqlConnPool::SqlConnPool(){}
SqlConnPool::~SqlConnPool(){
    ClosePool();
}
void SqlConnPool::Init(const char *host, const char *user, 
        const char *pwd, const char *dbName, int connSize){
    for(int i = 0; i < connSize; i++){
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if(!sql){
            // log
            continue;
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, 0, nullptr, 0);
        if(!sql){
            // log
            continue;
        }
        connQue_.push(sql);
    }
    sem_init(&semId_, 0, connQue_.size());
}
void SqlConnPool::ClosePool(){
    std::lock_guard<std::mutex> locker(mtx_);
    while(!connQue_.empty()){
        MYSQL *it = connQue_.front();
        connQue_.pop();
        mysql_close(it);
    }
}

MYSQL *SqlConnPool::GetConn(){
    MYSQL *sql;
    sem_wait(&semId_);
    {
        std::lock_guard<std::mutex> locker(mtx_);
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}
void SqlConnPool::FreeConn(MYSQL *sql){
    std::lock_guard<std::mutex> locker(mtx_);
    connQue_.push(sql);
    sem_post(&semId_);
}