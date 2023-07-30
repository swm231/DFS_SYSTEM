#include "server/webserver.h"

int main()
{
    WebServer server(80, 1000,
            "8.130.86.160", "swm_231", "123456", "WebServer",
            true, 1, 3600);
    server.startUp();
}
/*  端口号, timeout(ms), 
    mysql(username, passward, dbname) 
    日志开关, 日志等级,  cookie
*/