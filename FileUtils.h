#include <bits/stdc++.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

#ifndef FILE_UTILS
#define FILE_UTILS

class FileUtils {
    private:
    public:
        static bool doesFileExist(string fileName);
        static long getFileSize(string fileName);
};

#endif