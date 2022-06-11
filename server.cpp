// Server side C program to demonstrate Socket programming
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

class HttpRequest {
public:
    string method;
    string url;
    string httpVersion;
    string host;
    string userAgent;
    string accept;
};

#define PORT 8080

ulong getFileSize(string fileName)
{
    struct stat fileInfo;

    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd == -1 || fstat(fd, &fileInfo) == -1) {
        perror("Error while getting file size");
        exit(EXIT_FAILURE);
    }

    return fileInfo.st_size;
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
        if (chunkSize < 0) {
            perror("Erro reading chunk of a file");
            exit(EXIT_FAILURE);
        }

        sentChunkSize = write(socket, buffer, chunkSize);
		if (sentChunkSize == -1) {
            perror("Error sending chunk of a file");
            exit(EXIT_FAILURE);
        }
	} while(chunkSize > 0 && sentChunkSize > 0);

    close(fd);
}

string getExtension(string fileName) {
    return fileName.substr(fileName.find_last_of(".") + 1);
}

string getFileType(string fileName) {
    string extension = getExtension(fileName);
    const map<string, string> typeByExtensions = {
        {"html", "text/html"},
        {"png", "image/png"},
        {"js", "text/javascript"}
    }; 

    return typeByExtensions.at(extension);
}

void sendHttpResponseHeader(string fileName, int socket) {
    long fileSize = getFileSize(fileName);
    string str = "HTTP/1.1 200 OK\nContent-Type: " + getFileType(fileName) + "\nContent-Length: " +  to_string(fileSize) + "\n\n";
    write(socket , str.c_str() , (int)str.size());
    // send(socket, str.c_str(), (int)str.size(), 0);
}

vector<string> split(const char *str, const char delimiter) {
    vector<string> tokens;
    char *it = strtok((char*)str, &delimiter);

    while (it != NULL) {
        string token = it;
        tokens.push_back(token);
        it = strtok(NULL, &delimiter);
    }
    return tokens;
}

map<string, string> parseHttpRequest(char *buffer) {
    vector<string> lines = split(buffer, '\n');
    vector<vector<string>> words(lines.size());
    for (int i = 0; i < lines.size(); i++) {
        words[i] = split(lines[i].c_str(), ' ');
    }
    map<string, string> httpRequest = {
        {"Method", words[0][0]},
        {"Url", words[0][1]},
        {"Protocol Version", words[0][2]},
        {"Host", words[1][0]},
        {"User-Agent", words[2][0]},
    };

    return httpRequest;
}

map<string, string> getHttpRequest(int socket) {
    // como é um request http posso assumir que ele nao eh muito grande
    const int bufferSize = 30000;
    char buffer[bufferSize] = {0};
    int valread = read(socket , buffer, bufferSize);
    printf("%s\n", buffer);
    return parseHttpRequest(buffer);
}

void processGetRequest(int socket) {
    map<string, string> httpRequest = getHttpRequest(socket);
    string fileName = "." + (httpRequest["Url"] != "/" ? httpRequest["Url"] : "/index.html");
    sendHttpResponseHeader(fileName, socket);
    sendFile(fileName, socket);
}

void exit_routine()
{
	printf("Ending...\n");
}

int main()
{
    atexit(exit_routine);

    signal(SIGINT, exit);
	signal(SIGKILL, exit);
	signal(SIGQUIT, exit);

    int masterSocket, client_fd; long valread;
    vector<int> clientSockets;
    fd_set socketSet;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int option = 1;
    int maxFileDescriptor;
    
    // Creating socket file descriptor
    if ((masterSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        perror("Erro while setting master socket options");
		exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    
    if (bind(masterSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    int backlog = 10;  // backlog, defines the maximum number of pending connections that can be queued up before connections are refused
    if (listen(masterSocket, backlog) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("\n------------ Server Running on Port 8080 ------------\n\n");
        //  zera todo conjunto de sockets. Este conjunto é usado para monitorar os clientes conectados
		FD_ZERO(&socketSet);

		// coloca o socket do servidor(masterSocket) no conjunto
		FD_SET(masterSocket, &socketSet);
		maxFileDescriptor = masterSocket;

        // para cada cliente conectado, adiciona o socket do client no conjunto de sockets
		for (auto c : clientSockets)
		{
			if (c > 0) // verifica se o cliente está conectado.
				FD_SET(c, &socketSet);

			if (c > maxFileDescriptor) // guarda o número do maior file descriptor, usado no select
				maxFileDescriptor = c;
		}

        // bloqueia esperando atividade em pelo menos um dos sockets
		int activity = select(maxFileDescriptor + 1, &socketSet, NULL, NULL, NULL);
		
        // sus code
        if ((activity < 0) && (errno != EINTR))
		{
			// printf("select error");
			// sleep(1);
			continue;
		}

        // se há atividade no socket do servidor é um solicitação de conexão de um cliente
		if (FD_ISSET(masterSocket, &socketSet))
		{
            int newSocket;
			// aceita a conexão no socket do servidor e cria um novo socket
			if ((newSocket = accept(masterSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			printf("\nNew connection , socket fd is %d , ip is : %s , port : %d \n", newSocket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            clientSockets.push_back(newSocket); // adiciona o socket na lista de clientes
		}

        for (auto c : clientSockets)
		{
			// se há atividade no socket do clientes conectados, então é uma solicitação de comando
			if (FD_ISSET(c, &socketSet))
			{
                processGetRequest(c);
			}
        }

        // if ((client_fd = accept(masterSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        // {
        //     perror("In accept");
        //     exit(EXIT_FAILURE);
        // }

        // processGetRequest(client_fd);
    }

    // O desejado eh que o cliente feche a sua conexao automaticamente
    close(client_fd);
    close(masterSocket);
    return 0;
}