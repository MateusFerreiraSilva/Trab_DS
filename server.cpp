// Server side C program to demonstrate Socket programming
#include <bits/stdc++.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include<fcntl.h> 
#include<errno.h> 

using namespace std;


#define PORT 8083

long getFileSize(string fileName)
{

    FILE *filePointer = fopen(fileName.c_str(), "rb");
    long fileSize = 0;
	if (filePointer) {
        fseek(filePointer, 0L, SEEK_END);
        fileSize = ftell(filePointer);
        rewind(filePointer);
        fclose(filePointer);
    }
	return fileSize;
}

void sendFile(string fileName, int socket)
{
    const int bufferSize = 32767; // max send size, default limit of send system call
    char buffer[bufferSize];
    int chunkSize = 0, sentChunkSize = 0;

    int fd = open(fileName.c_str(), O_RDONLY);

    if (fd == -1) {
        perror("Internal Server Error Reading File");
        exit(1);
    }

	int sentDataSize, offset = 0;
	do {
        chunkSize = read(fd, buffer, bufferSize);
        printf("%s\n", buffer);
        if (chunkSize > 0) {
		    sentChunkSize = write(socket, buffer, chunkSize);
            printf("%d\n", sentChunkSize);
        }
		if (sentChunkSize == -1)
            break;
	} while(chunkSize > 0 && sentChunkSize > 0);

    close(fd);
}


void sendHttpResponseHeader(string fileName, int socket) {
    long fileSize = getFileSize(fileName);
    string str = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " +  to_string(fileSize) + "\n\n";
    write(socket , str.c_str() , (int)str.size());
    // send(socket, str.c_str(), (int)str.size(), 0);
}


void getHttpRequest(int socket) {
    // como é um request http posso assumir que ele nao eh muito grande
    const int bufferSize = 30000;
    char buffer[bufferSize] = {0};
    int valread = read(socket , buffer, bufferSize);
    printf("%s\n", buffer);
}

void processGetRequest(string fileName, int socket) {
    getHttpRequest(socket);
    sendHttpResponseHeader(fileName, socket);
    sendFile(fileName, socket);
}

int main()
{
    int server_fd, new_socket; long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        processGetRequest("index.html", new_socket);
    }
    return 0;
}