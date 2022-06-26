#include "HttpServer.h"

// TO DO caso uma thread de erro não matar o programa
// TO DO garantir que a conexão seja fechada se não minha thread fica travada (ex: contador de erro quando recv retorna 0, ou seja recv pode retorna 0 no max N vezes se não o processo é encerrado)
// TO DO programa trava depois de um http

void HttpServer::sendFile(int socket, string fileName)
{
    const int bufferSize = threadPool->getSendBufferSize();
    char* buffer = threadPool->getSendBuffer();
    int chunkSize = 0, sentChunkSize = 0;

    int fd = open(fileName.c_str(), O_RDONLY);

    if (fd == -1) {
        throw runtime_error("Error while opening file to send");
    }

	int sentDataSize, offset = 0;
	while (true) {
        chunkSize = read(fd, buffer, bufferSize);

        if (chunkSize == 0) { // EOF
            close(fd);
            return;
        } else if (chunkSize == -1) {
            close(fd);
            throw runtime_error("Error while reading file chunk");
        } else {
            sentChunkSize = send(socket, buffer, chunkSize, MSG_NOSIGNAL);
            if (sentChunkSize == -1) {
                close(fd);
                throw runtime_error("Error while sending chunk");
            }
        }
	}
}

void HttpServer::sendNotFoundResponse(int socket) {
    string notFoundMessage = "Ops! This doesn't exist.";
    string notFoundMessageSize = to_string(notFoundMessage.size());
    string httpResponse = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: "
                            + notFoundMessageSize + "\n\n"
                            + notFoundMessage;

    if(send(socket, httpResponse.c_str(), (int)httpResponse.size(), MSG_NOSIGNAL) == -1) {
        throw runtime_error("Error while sending http response");
    }
}

void HttpServer::sendInternalServerErrorResponse(int socket) {
    string internalServerErrorMessage = "500 Internal Server Error";
    string internalServerErrorMessageSize = to_string(internalServerErrorMessage.size());
    string httpResponse = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\nContent-Length: "
                            + internalServerErrorMessageSize + "\n\n"
                            + internalServerErrorMessage;

     if(send(socket, httpResponse.c_str(), (int)httpResponse.size(), MSG_NOSIGNAL) == -1) {
        throw runtime_error("Error while sending http response");
    }
}

void HttpServer::sendMethodNotAllowedResponse(int socket) {
    string methodNotAllowedMessage = "405 Method Not Allowed";
    string methodNotAllowedMessageSize = to_string(methodNotAllowedMessage.size());
    string httpResponse = "HTTP/1.1 Internal Server Error\nContent-Type: text/plain\nContent-Length: "
                            + methodNotAllowedMessageSize + "\n\n"
                            + methodNotAllowedMessage;

     if(send(socket, httpResponse.c_str(), (int)httpResponse.size(), MSG_NOSIGNAL) == -1) {
        throw runtime_error("Error while sending http response");
    }
}

void HttpServer::sendOkResponseHeader(int socket, string fileName) {
    long fileSize = FileUtils::getFileSize(fileName);
    string httpResponse = "HTTP/1.1 200 OK\nContent-Type: "
                            + StringUtils::getFileType(fileName)
                            + "\nContent-Length: "
                            +  to_string(fileSize)
                            + "\n\n";

    if(send(socket, httpResponse.c_str(), (int)httpResponse.size(), MSG_NOSIGNAL) == -1) {
        throw runtime_error("Error while sending http response");
    }
}

void HttpServer::sendOkResponse(int socket, string fileName) {
    sendOkResponseHeader(socket, fileName);
    sendFile(socket, fileName);
}

void HttpServer::sendHttpResponse(string fileName, int socket) {
    if (FileUtils::getFileSize(fileName)) {
        sendOkResponse(socket, fileName);
    } else {
        sendNotFoundResponse(socket);
    }
}

map<string, string> HttpServer::getHttpRequest(int socket) {
    int bufferSize = threadPool->getRecvBufferSize();
    char* buffer = threadPool->getRecvBuffer();
    map<string, string> httpRequest;

    // morre aqui no caso do navegador, como navegador não manda nada a thread fica aqui esperando...
    int httpRequestSize = recv(socket , buffer, bufferSize, 0);
    if (httpRequestSize > 0) {
        // printf("%s\n", buffer);
        httpRequest = StringUtils::parseHttpRequest(buffer, bufferSize);
    } else if (httpRequestSize <= 0) {
        if (httpRequestSize == -1) {
            // perror("Disconnecting becausa a error or a timeout\n");
        }

        disconnectClient(socket);
    }

    return httpRequest;
}

void HttpServer::handleError(int socket) {
    try {
        sendInternalServerErrorResponse(socket);
    } catch (...) {}
    
    disconnectClient(socket);
}

void HttpServer::processHttpRequest(int socket) {
    // printf("Processing Http Request To Socket: %d\n", socket);
    try {
        setListenTimeout(socket);
        map<string, string> httpRequest;
        while (true) {
            httpRequest = getHttpRequest(socket);
            if (httpRequest.empty()) {
                break;
            } else if (httpRequest["Method"] != "GET") {
                sendMethodNotAllowedResponse(socket);
            }
            else {
                string fileName = "." + (httpRequest["Url"] != "/" ? httpRequest["Url"] : "/index.html");
                sendHttpResponse(fileName, socket);
            }
        }
    }
    catch(const std::runtime_error& re) {
        handleError(socket);
        cerr << "Runtime error: " << re.what() << std::endl;
    }
    catch(const std::exception& ex) {
        handleError(socket);
        cerr << "Error occurred: " << ex.what() << std::endl;
    }
    catch(...)
    {
        handleError(socket);
    }
    // printf("Request was completed\n");
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
    // printf("Disconnection , socket fd is %d , ip is : %s , port : %d \n", socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    close(socket);
}

int HttpServer::acceptConnection() {
    struct sockaddr_in clientAddress;
    int clientAddrlen = sizeof(clientAddress);
    int socket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddrlen);
    // printf("\nNew connection , socket fd is %d , ip is : %s , port : %d \n", socket, inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
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