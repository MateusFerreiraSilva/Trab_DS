#include "StringUtils.h"

string StringUtils::getExtension(string fileName) {
    return fileName.substr(fileName.find_last_of(".") + 1);
}

string StringUtils::getFileType(string fileName) {
    string extension = getExtension(fileName);
    
    if (typeByExtensions.find(extension) == typeByExtensions.end()) // if extension was not found
        return "application/octet-stream"; // see RFC2616

    return typeByExtensions.at(extension);
}

map<string, string> StringUtils::parseHttpRequest(char *buffer, int bufferSize) {
    string request = "";
    for (int i = 0; i < bufferSize; i++) {
        if (buffer[i] != '\r')
            request.push_back(buffer[i]);
        else
            break;
    }

    string method, url;
    int i;
    for (i = 0; i < request.size(); i++) {
        
        if (request[i] != ' ')
            method.push_back(request[i]);
        else
            break;
    }

    for (int j = i + 1; j < request.size(); j++) {
        if (request[j] != ' ')
            url.push_back(request[j]);
        else
            break;
    }

    map<string, string> httpRequest = {
        {"Method", method},
        {"Url", url}
    };
    
    return httpRequest;
}