#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

#ifndef STRING_UTILS
#define STRING_UTILS

class StringUtils {
    private:
        inline static const map<string, string> typeByExtensions = {
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

        static string getExtension(string fileName);
    public:
        static map<string, string> parseHttpRequest(char *buffer, int bufferSize);
        static string getFileType(string fileName);
};

#endif