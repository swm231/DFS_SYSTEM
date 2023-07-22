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

int main()
{
    // 端口号, timeout(ms)
    WebServer server(8888, 30000);
    server.startUp();
}