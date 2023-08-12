#include "sqlconnpool.h"

SqlConnPool::SqlConnPool() : MAX_CONN_(0), FreshenThread_(nullptr), stop_(true){}
SqlConnPool::~SqlConnPool(){}

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
    stop_ = false;
}
void SqlConnPool::ClosePool(){
    std::lock_guard<std::mutex> locker(mtx_);
    stop_ = true;
    MAX_CONN_ = 0;
    while(!connQue_.empty()){
        MYSQL *it = connQue_.front();
        connQue_.pop();
        mysql_close(it);
    }
    if(FreshenThread_->joinable())
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
    size_t Waittime = 0;
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if(stop_)
            break;
        Waittime += 2;
        if(Waittime < 28000)
            continue;
        Waittime = 0;
        LOG_INFO("[sqlpool] 刷新连接池");
        std::lock_guard<std::mutex> locker(mtx_);
        for(int i = 0; i < MAX_CONN_; i++){
            MYSQL *sql = GetConn();
            mysql_reset_connection(sql);
            FreeConn(sql);
        }
    }
}