#include "sqlconnpool.h"

SqlConnPool::SqlConnPool() : MAX_CONN_(0), FreshenThread_(nullptr){}
SqlConnPool::~SqlConnPool(){
    ClosePool();
}
void SqlConnPool::Init(const char *host, const char *user, 
        const char *pwd, const char *dbName, int connSize){
    for(int i = 0; i < connSize; i++){
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if(!sql){
            LOG_ERROR("[sqlpool] MYSQL初始化失败");
            continue;
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, 3306, nullptr, 0);
        if(!sql){
            LOG_ERROR("[sqlpool] mysql连接失败");
            continue;
        }
        connQue_.push(sql);
    }
    MAX_CONN_ = connQue_.size();
    sem_init(&semId_, 0, connQue_.size());
    FreshenThread_ = std::make_unique<std::thread>(&SqlConnPool::StartFresh, this);
}
void SqlConnPool::ClosePool(){
    std::lock_guard<std::mutex> locker(mtx_);
    while(!connQue_.empty()){
        MYSQL *it = connQue_.front();
        connQue_.pop();
        mysql_close(it);
    }
    FreshenThread_->join();
}

MYSQL *SqlConnPool::GetConn(){
    MYSQL *sql;
    if(connQue_.empty())
        LOG_WARN("[sqlpool] 线程池忙!!");
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

void SqlConnPool::StartFresh(){
    SqlConnPool::Instance().Freshen_();
}

void SqlConnPool::Freshen_(){
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(28000));
        LOG_INFO("[sqlpool] 刷新连接池");
        std::lock_guard<std::mutex> locker(mtx_);
        for(int i = 0; i < MAX_CONN_; i++){
            MYSQL *sql = GetConn();
            mysql_reset_connection(sql);
            FreeConn(sql);
        }
    }
}