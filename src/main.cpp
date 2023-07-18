#include "server/webserver.h"

int main()
{
    WebServer server(8888);
    server.startUp();
}