#include "server/webserver.h"

ThreadPool& globalThreadPool(){
    static ThreadPool globalThreadPool;
    return globalThreadPool;
}
Epoll& globalEpoll(){
    static Epoll globalEpoll(512);
    return globalEpoll;
}

int main()
{
    WebServer server(8888);
    server.startUp();
}