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

using namespace std;

#ifndef HTTP_SERVER
#define HTTP_SERVER

#define PORT 8080
#define MAX_HTTP_GET_MESSAGE_SIZE 8192

class HttpServer {
    private:
        int masterSocket;
        set<int> clientSockets;
        fd_set socketSet;
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        int option = 1;
        int maxFileDescriptor;
        int backlog = 50;  // backlog, defines the maximum number of pending connections that can be queued up before connections are refused
        char* httpRequestBuffer[MAX_HTTP_GET_MESSAGE_SIZE];

        void setConfigs();
        void processGetRequest(int socket);
        map<string, string> getHttpRequest(int socket);
        map<string, string> parseHttpRequest(char *buffer);
        vector<string> split(const char *str, const char delimiter);
        void sendHttpResponseHeader(string fileName, int socket);
        string getFileType(string fileName);
        string getExtension(string fileName);
        void sendFile(string fileName, int socket);
        ulong getFileSize(string fileName);
        void checkForClientDisconnections();
        void acceptConnections();
        int getMaxFileDescriptor();
        void processRequests();

    public:
        /*
            Http does not limit the size of a GET request, but most of the browser have a limit between 2K~8K bytes.
        */
        HttpServer();
        void Start();
        void Listen();

        static void ExitRoutine();
};

#endif