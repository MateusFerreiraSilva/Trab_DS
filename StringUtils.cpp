#include "StringUtils.h"

string StringUtils::getExtension(string fileName) {
    return fileName.substr(fileName.find_last_of(".") + 1);
}

string StringUtils::getFileType(string fileName) {
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

// pass slice = 0 to request all slices 
vector<string> StringUtils::split(const char *str, const char delimiter, const int slices) {
    int currSlices = 0;
    vector<string> tokens;
    char *it = strtok((char*)str, &delimiter);

    while (it != NULL) {
        string token = it;
        tokens.push_back(token);
        currSlices++;

        if (currSlices == slices) {
            break;
        }

        it = strtok(NULL, &delimiter);
    }

    return tokens;
}

map<string, string> StringUtils::parseHttpRequest(char *buffer) {
    vector<string> request = split(buffer, '\n', 1);
    vector<string> words = split(request.front().c_str(), ' ', 0);
    map<string, string> httpRequest = {
        {"Method", words[0]},
        {"Url", words[1]}
        // {"Version", words[2]}
    };
    
    return httpRequest;
}