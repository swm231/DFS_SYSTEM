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
    // 端口号, timeout(ms), mysql(username, passward, dbname) 
    WebServer server(8888, 30000, "root", "123456", "WebServer");
    server.startUp();
}