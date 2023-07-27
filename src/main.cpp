#include "server/webserver.h"

ThreadPool& globalThreadPool(){
    static ThreadPool globalThreadPool;
    return globalThreadPool;
}
Epoll& globalEpoll(){
    static Epoll globalEpoll(512);
    return globalEpoll;
}
HeapTimer& globalHeapTimer(){
    static HeapTimer globalHeapTimer;
    return globalHeapTimer;
}
SqlConnPool& globalSqlConnPool(){
    static SqlConnPool globalSqlConnPool;
    return globalSqlConnPool;
}
Log& globalLog(){
    static Log globalLog;
    return globalLog;
}

int main()
{
    WebServer server(80, 60000,
            "8.130.86.160", "swm_231", "123456", "WebServer",
            true, 1, 3600);
    server.startUp();
}
/*  端口号, timeout(ms), 
    mysql(username, passward, dbname) 
    日志开关, 日志等级,  cookie
*/