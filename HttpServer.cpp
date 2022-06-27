#include "HttpServer.h"

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

    printf("\n------------ Server Running on Port %d ------------\n", PORT);
    printf("\nPress Crtl + C to stop...\n");
}

void HttpServer::setConfigs() {
    atexit(exitRoutine);
    signal(SIGINT, exit);
}

void HttpServer::exitRoutine()
{
	printf("Stopping Http Server...\n");
    threadPool->stop();
    close(serverSocket);
}

void HttpServer::run() {
    while(true) {
        int new_connection = acceptConnection();
        queueRequest(new_connection);
    }
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

void HttpServer::disconnectClient(int socket) {
    // printf("Disconnection , socket fd is %d , ip is : %s , port : %d \n", socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    close(socket);
}

void HttpServer::queueRequest(int socket) {
    threadPool->queueJob(processHttpRequest, socket);
}

void HttpServer::processHttpRequest(int socket) {
    // printf("Processing Http Request To Socket: %d\n", socket);
    try {
        setRecvTimeout(socket);
        map<string, string> httpRequest;
        while (!threadPool->shouldStop()) {
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

void HttpServer::setRecvTimeout(int socket) {
    struct timeval tv;
    tv.tv_sec = secondsToTimeout;
    tv.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

void HttpServer::handleError(int socket) {
    try {
        sendInternalServerErrorResponse(socket);
    } catch (...) {
        printf("Error sending internal server error response");
    }
    
    disconnectClient(socket);
}

map<string, string> HttpServer::getHttpRequest(int socket) {
    int bufferSize = threadPool->getRecvBufferSize();
    char* buffer = threadPool->getRecvBuffer();
    map<string, string> httpRequest;

    int httpRequestSize = recv(socket , buffer, bufferSize, 0); // timeout of 1 second
    if (httpRequestSize > 0) {
        // printf("%s\n", buffer);
        httpRequest = StringUtils::parseHttpRequest(buffer, bufferSize);
    } else if (httpRequestSize <= 0) {
        disconnectClient(socket);
    }

    return httpRequest;
}

void HttpServer::sendHttpResponse(string fileName, int socket) {
    if (FileUtils::doesFileExist(fileName)) {
        sendOkResponse(socket, fileName);
    } else {
        sendNotFoundResponse(socket);
    }
}

void HttpServer::sendOkResponse(int socket, string fileName) {
    sendOkResponseHeader(socket, fileName);
    sendFile(socket, fileName);
}

void HttpServer::sendOkResponseHeader(int socket, string fileName) {
    int retries = 0;
    const int maxNumOfRetries = 5;
    long fileSize = FileUtils::getFileSize(fileName);
    string httpResponse = "HTTP/1.1 200 OK\nContent-Type: "
                            + StringUtils::getFileType(fileName)
                            + "\nContent-Length: "
                            +  to_string(fileSize)
                            + "\n\n";

    while (send(socket, httpResponse.c_str(), (int)httpResponse.size(), MSG_NOSIGNAL) == -1) {
        retries++;
        if (retries < maxNumOfRetries)
            sleep(1);
        else
            throw runtime_error("Error while sending http response");
    }
}

void HttpServer::sendFile(int socket, string fileName) {
    int retries = 0;
    const int maxNumOfRetries = 10;

    const int bufferSize = threadPool->getSendBufferSize();
    char* buffer = threadPool->getSendBuffer();
    int chunkSize = 0, sentChunkSize = 0;

    int fd = open(fileName.c_str(), O_RDONLY);
    while (fd == -1) {
        retries++;
        if (retries < maxNumOfRetries) {
            sleep(1);
            fd = open(fileName.c_str(), O_RDONLY);
        } else {
            throw runtime_error("Error while opening file to send");
        }
    } // succeeded reading the file

	while (true) {
        chunkSize = read(fd, buffer, bufferSize);
        while (chunkSize == -1) {
            retries++;
            if (retries < maxNumOfRetries) {
                sleep(1);
                chunkSize = read(fd, buffer, bufferSize);
            } else {
                close(fd);
                throw runtime_error("Error while reading file chunk");
            }
        } // succeeded reading a chung of the file

        if (chunkSize == 0) { // EOF
            close(fd);
            return; // has finished this file transfer
        } else { // chunkSize > 0
            sentChunkSize = send(socket, buffer, chunkSize, MSG_NOSIGNAL);
            while (sentChunkSize == -1) {
                retries++;
                if (retries < maxNumOfRetries) {
                    sleep(1);
                    sentChunkSize = send(socket, buffer, chunkSize, MSG_NOSIGNAL);
                } else {
                    close(fd);
                    throw runtime_error("Error while sending chunk");
                }
            }
        }
	}
}

void HttpServer::sendErrorResponseHeader(int socket, string message) {
    int retries = 0;
    const int maxNumOfRetries = 5;

    string messageSize = to_string(message.size());
    string httpResponse = "HTTP/1.1 " + message + "\nContent-Type: text/plain\nContent-Length: "
                            + messageSize + "\n\n"
                            + message;

    while (send(socket, httpResponse.c_str(), (int)httpResponse.size(), MSG_NOSIGNAL) == -1) {
        retries++;
        if (retries < maxNumOfRetries)
            sleep(1);
        else
            throw runtime_error("Error while sending error http response");
    }
}

void HttpServer::sendNotFoundResponse(int socket) {
    string notFoundMessage = "404 Not Found";
    sendErrorResponseHeader(socket, notFoundMessage);
}

void HttpServer::sendMethodNotAllowedResponse(int socket) {
    string methodNotAllowedMessage = "405 Method Not Allowed";
    sendErrorResponseHeader(socket, methodNotAllowedMessage);
}

void HttpServer::sendInternalServerErrorResponse(int socket) {
    string internalServerErrorMessage = "500 Internal Server Error";
    sendErrorResponseHeader(socket, internalServerErrorMessage);
}