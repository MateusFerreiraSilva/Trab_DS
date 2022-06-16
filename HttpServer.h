#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "ThreadPool.h"

#ifndef HTTP_SERVER
#define HTTP_SERVER

#define PORT 8080
#define MAX_HTTP_GET_MESSAGE_SIZE 8192
#define MAX_CLIENTS 5000

class HttpServer {
    private:
        inline static int masterSocket;
        inline static struct sockaddr_in address;
        inline static const int addrlen = sizeof(address);
        inline static const int option = 1;
        inline static const int backlog = 50;  // backlog, defines the maximum number of pending connections that can be queued up before connections are refused
        inline static char* httpRequestBuffer[MAX_HTTP_GET_MESSAGE_SIZE];
        inline static ThreadPool *threadPool;

        static void setConfigs();
        static void processGetRequest(int socket);
        static map<string, string> getHttpRequest(int socket);
        static map<string, string> parseHttpRequest(char *buffer);
        static vector<string> split(const char *str, const char delimiter);
        static void sendHttpResponseHeader(string fileName, int socket);
        static string getFileType(string fileName);
        static string getExtension(string fileName);
        static void sendFile(string fileName, int socket);
        static ulong getFileSize(string fileName);
        static void disconnectClient(int socket);
        static int acceptConnection(); // return socket with new connection
        static void queueRequest(int socket);

    public:
        /*
            Http does not limit the size of a GET request, but most of the browser have a limit between 2K~8K bytes.
        */
        static void start();
        static void run();

        static void exitRoutine();
};

#endif