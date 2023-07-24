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

int main()
{
    // 端口号, timeout(ms), cookie, mysql(username, passward, dbname) 
    WebServer server(8888, 60000, 3600, "8.130.86.160", "swm_231", "123456", "WebServer");
    server.startUp();
}