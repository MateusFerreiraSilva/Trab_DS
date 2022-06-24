#include "HttpServer.h"

// TO DO caso uma thread de erro não matar o programa
// TO DO garantir que a conexão seja fechada se não minha thread fica travada (ex: contador de erro quando recv retorna 0, ou seja recv pode retorna 0 no max N vezes se não o processo é encerrado)
// TO DO programa trava depois de um http

ulong HttpServer::getFileSize(string fileName)
{
    struct stat fileInfo;

    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd == -1 || fstat(fd, &fileInfo) == -1) {
        perror("Error while getting file size");
        exit(EXIT_FAILURE);
    }
    close(fd);

    return fileInfo.st_size;
}

void HttpServer::sendFile(string fileName, int socket)
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

        sentChunkSize = send(socket, buffer, chunkSize, 0);
		if (sentChunkSize == -1) {
            perror("Error sending chunk of a file");
            exit(EXIT_FAILURE);
        }
	} while(chunkSize > 0 && sentChunkSize > 0);

    close(fd);
}

string HttpServer::getExtension(string fileName) {
    return fileName.substr(fileName.find_last_of(".") + 1);
}

string HttpServer::getFileType(string fileName) {
    string extension = getExtension(fileName);
    const map<string, string> typeByExtensions = {
        {"html", "text/html"},
        {"png", "image/png"},
        {"js", "text/javascript"},
        {"ico", "image/vnd.microsoft.icon"}
    };
    // if not found
    if (typeByExtensions.find(extension) == typeByExtensions.end())
        return "application/octet-stream"; // see RFC2616

    return typeByExtensions.at(extension);
}

void HttpServer::buildHttpResponse() {
    
}

void HttpServer::sendHttpResponseHeader(string fileName, int socket) {
    long fileSize = getFileSize(fileName);
    string str = "HTTP/1.1 200 OK\nContent-Type: " + getFileType(fileName) + "\nContent-Length: " +  to_string(fileSize) + "\n\n";
    // write(socket , str.c_str() , (int)str.size());
    send(socket, str.c_str(), (int)str.size(), 0);
}

vector<string> HttpServer::split(const char *str, const char delimiter) {
    vector<string> tokens;
    char *it = strtok((char*)str, &delimiter);

    while (it != NULL) {
        string token = it;
        tokens.push_back(token);
        it = strtok(NULL, &delimiter);
    }
    return tokens;
}

map<string, string> HttpServer::parseHttpRequest(char *buffer) {
    vector<string> lines = split(buffer, '\n');
    vector<vector<string>> words(lines.size());
    for (int i = 0; i < lines.size(); i++) {
        words[i] = split(lines[i].c_str(), ' ');
    }
    // map<string, string> httpRequest = {
    //     {"Method", words[0][0]},
    //     {"Url", words[0][1]},
    //     {"Protocol Version", words[0][2]},
    //     {"Host", words[1][0]},
    //     {"User-Agent", words[2][0]},
    // };
    map<string, string> httpRequest = {
        {"Method", words[0][0]},
        {"Url", words[0][1]}
    };
    return httpRequest;
}

map<string, string> HttpServer::getHttpRequest(int socket) {
    /*TO DO Loop de leitura para arquivo grandes*/
    map<string, string> httpRequest;
    char buffer[MAX_HTTP_GET_MESSAGE_SIZE];
    int httpRequestSize = recv(socket , buffer, MAX_HTTP_GET_MESSAGE_SIZE, 0);
    if (httpRequestSize > 0) {
        // printf("%s\n", buffer);
        httpRequest = parseHttpRequest(buffer);
    } else if (httpRequestSize == 0) {
       disconnectClient(socket);
    } else {
        perror("Error reading GET Method");
    }
    return httpRequest;
}

void HttpServer::processGetRequest(int socket) {
    printf("Processing Http Request To Socket: %d\n", socket);
    try {
        map<string, string> httpRequest;
        while (true) {
            httpRequest = getHttpRequest(socket);
            if (!httpRequest.empty() && httpRequest["Method"] == "GET") {
                string fileName = "." + (httpRequest["Url"] != "/" ? httpRequest["Url"] : "/index.html");
                sendHttpResponseHeader(fileName, socket);
                sendFile(fileName, socket);
            } else {
                break;
            }
        }
    } catch (...) {
       printf("Something went wrong processing http request\n");
    }
    printf("Request was completed\n");
}

void HttpServer::exitRoutine()
{
	printf("Stopping Http Server...\n");
    threadPool->stop();
}

void HttpServer::setConfigs() {
    atexit(exitRoutine);
    signal(SIGINT, exit);
	signal(SIGKILL, exit);
	signal(SIGQUIT, exit);
}

void HttpServer::start() {
    setConfigs();
    threadPool = new ThreadPool();
    threadPool->start();

    // Creating socket file descriptor
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    int option = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        perror("Erro while setting master socket options");
		exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
    
    memset(address.sin_zero, '\0', sizeof address.sin_zero);
    
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("In bind");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, backlog) < 0)
    {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    printf("\n------------ Server Running on Port 8080 ------------\n\n");
}

void HttpServer::disconnectClient(int socket) {
    printf("\nDisconnection , socket fd is %d , ip is : %s , port : %d \n", socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    close(socket);
}

int HttpServer::acceptConnection() {
    struct sockaddr_in clientAddress;
    int clientAddrlen = sizeof(clientAddress);
    int socket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddrlen);
    printf("\nNew connection , socket fd is %d , ip is : %s , port : %d \n", socket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
    if (socket < 0) {
        perror("Error acception connection");
    }
    return socket;
}

void HttpServer::queueRequest(int socket) {
    threadPool->queueJob(processGetRequest, socket);
}

void HttpServer::run() {
    while(true) {
        int new_connection = acceptConnection();
        queueRequest(new_connection);
    }
}