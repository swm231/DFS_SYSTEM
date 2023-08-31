#include "server/trackerserver.h"

TrackerServer server;
static void Close(int signum){
    server.CloseServer(signum);
    ThreadPool::Instance().Shutdown();
    SqlConnPool::Instance().ClosePool();
}
int main()
{
    signal(SIGINT, static_cast<sighandler_t>(Close));
    server.StartUp();

    return 0;
}