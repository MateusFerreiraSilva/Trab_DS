#include "HttpServer.h"

// TO DO caso uma thread de erro não matar o programa
// TO DO garantir que a conexão seja fechada se não minha thread fica travada (ex: contador de erro quando recv retorna 0, ou seja recv pode retorna 0 no max N vezes se não o processo é encerrado)
// TO DO programa trava depois de um http

bool HttpServer::doesFileExist(string fileName) {
    return access(fileName.c_str(), F_OK) != -1;
}

ulong HttpServer::getFileSize(string fileName)
{
    struct stat fileInfo;
    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd == -1 || fstat(fd, &fileInfo) == -1) {
        perror("Error while getting file size");
        exit(EXIT_FAILURE);
    } else {
        close(fd);
    }

    return fileInfo.st_size;
}

void HttpServer::sendFile(int socket, string fileName)
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
        {"txt", "txt/plain"},
        {"html", "text/html"},
        {"css", "text/css"},
        {"js", "text/javascript"},
        {"ico", "image/vnd.microsoft.icon"},
        {"json", "application/json"},
        {"csv", "text/csv"},
        {"xml", "application/xml"},
        {"pdf", "application/pdf"},
        {"png", "image/png"},
        {"jpeg", "image/jpeg"},
        {"jpg", "image/jpeg"}
    };
    
    if (typeByExtensions.find(extension) == typeByExtensions.end()) // if extension was not found
        return "application/octet-stream"; // see RFC2616

    return typeByExtensions.at(extension);
}

void HttpServer::sendNotFoundResponse(int socket) {
    string notFoundMessage = "Ops! This doesn't exist.";
    string notFoundMessageSize = to_string(notFoundMessage.size());
    string httpResponse = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: "
                            + notFoundMessageSize + "\n\n"
                            + notFoundMessage;

    send(socket, httpResponse.c_str(), (int)httpResponse.size(), 0);
}

void HttpServer::sendBadRequestResponse(int socket) {
    string badRequestMessage = "400 Bad Request";
    string badRequestMessageSize = to_string(badRequestMessage.size());
    string httpResponse = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: "
                            + badRequestMessageSize + "\n\n"
                            + badRequestMessage;

    send(socket, httpResponse.c_str(), (int)httpResponse.size(), 0);
}

void HttpServer::sendOkResponseHeader(int socket, string fileName) {
    long fileSize = getFileSize(fileName);
    string httpResponse = "HTTP/1.1 200 OK\nContent-Type: "
                            + getFileType(fileName)
                            + "\nContent-Length: "
                            +  to_string(fileSize)
                            + "\n\n";

    send(socket, httpResponse.c_str(), (int)httpResponse.size(), 0);
}

void HttpServer::sendOkResponse(int socket, string fileName) {
    sendOkResponseHeader(socket, fileName);
    sendFile(socket, fileName);
}

void HttpServer::sendHttpResponse(string fileName, int socket) {
    if (doesFileExist(fileName)) {
        sendOkResponse(socket, fileName);
    } else {
        sendNotFoundResponse(socket);
    }
}

map<string, string> HttpServer::getHttpRequest(int socket) {
    char buffer[MAX_HTTP_GET_MESSAGE_SIZE]; // TO DO alterar para malloc
    map<string, string> httpRequest;

    // morre aqui no caso do navegador, como navegador não manda nada a thread fica aqui esperando...
    int httpRequestSize = recv(socket , buffer, MAX_HTTP_GET_MESSAGE_SIZE, 0);
    if (httpRequestSize > 0) {
        // printf("%s\n", buffer);
        httpRequest = StringUtils::parseHttpRequest(buffer);
    } else if (httpRequestSize <= 0) {
        if (httpRequestSize == -1) {
            perror("Disconnecting becausa a error or a timeout\n");
        }

        disconnectClient(socket);
    }

    return httpRequest;
}

void HttpServer::processHttpRequest(int socket) {
    printf("Processing Http Request To Socket: %d\n", socket);
    try {
        setListenTimeout(socket);
        map<string, string> httpRequest;
        while (true) {
            httpRequest = getHttpRequest(socket);
            if (httpRequest.empty()) {
                break;
            }
            else if (httpRequest["Method"] == "GET") {
                string fileName = "." + (httpRequest["Url"] != "/" ? httpRequest["Url"] : "/index.html");
                sendHttpResponse(fileName, socket);
            } else {
                sendBadRequestResponse(socket);
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

void HttpServer::setListenTimeout(int socket) {
    struct timeval tv;
    tv.tv_sec = secondsToTimeout;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
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
    address.sin_port = htons(PORT);
    
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
    printf("Disconnection , socket fd is %d , ip is : %s , port : %d \n", socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
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
    threadPool->queueJob(processHttpRequest, socket);
}

void HttpServer::run() {
    while(true) {
        int new_connection = acceptConnection();
        queueRequest(new_connection);
    }
}