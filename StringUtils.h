#include <bits/stdc++.h>
using namespace std;

#ifndef STRING_UTILS
#define STRING_UTILS

class StringUtils {
    private:
        static string getExtension(string fileName);
    public:
        static map<string, string> parseHttpRequest(char *buffer, int bufferSize);
        static string getFileType(string fileName);
};

#endif