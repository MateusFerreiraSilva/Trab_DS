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
#define MAX_CLIENTS 5000
// limit the http request max size
#define MAX_HTTP_GET_MESSAGE_SIZE 1000 * 1024 * 2 // 2MB

class HttpServer {
    private:
        inline static int serverSocket;
        inline static struct sockaddr_in address;
        inline static const int backlog = 10;  // backlog, defines the maximum number of pending connections that can be queued up before connections are refused
        inline static ThreadPool *threadPool;

        static void setConfigs();
        static void processHttpRequest(int socket);
        static map<string, string> getHttpRequest(int socket);
        static map<string, string> parseHttpRequest(char *buffer);
        static vector<string> split(const char *str, const char delimiter);
        static void sendHttpResponse(string fileName, int socket);
        static string getFileType(string fileName);
        static string getExtension(string fileName);
        static void sendFile(int socket, string fileName);
        static ulong getFileSize(string fileName);
        static void disconnectClient(int socket);
        static int acceptConnection(); // return socket with new connection
        static void queueRequest(int socket);
        static void buildHttpResponse();
        static bool doesFileExist(string fileName);
        static void sendNotFoundResponse(int socket);
        static void sendBadRequestResponse(int socket);
        static void sendOkResponseHeader(int socket, string fileName);
        static void sendOkResponse(int socket, string fileName);

    public:
        /*
            Http does not limit the size of a GET request, but most of the browser have a limit between 2K~8K bytes.
        */
        static void start();
        static void run();

        static void exitRoutine();
};

#endif