#include "server/storageserver.h"


StorageServer server;
static void Close(int signum){
    server.CloseServer(signum);
    ThreadPool::Instance().Shutdown();
    SqlConnPool::Instance().ShutDown();
    Log::Instance().ShutDown();
}
int main(){
    signal(SIGINT, static_cast<sighandler_t>(Close));
    server.StartUp();

    return 0;
}