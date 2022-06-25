#include <bits/stdc++.h>
using namespace std;

#ifndef STRING_UTILS
#define STRING_UTILS

class StringUtils {
    private:
        static vector<string> split(const char *str, const char delimiter, const int slices);
    public:
        static map<string, string> parseHttpRequest(char *buffer);
};

#endif