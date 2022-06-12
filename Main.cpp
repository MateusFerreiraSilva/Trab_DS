#include "HttpServer.h"

int main()
{    
    HttpServer *server = new HttpServer();
    server->Start();
    server->Listen();
    return 0;
}