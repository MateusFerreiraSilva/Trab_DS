#include "FileUtils.h"

bool FileUtils::doesFileExist(string fileName) {
    return access(fileName.c_str(), F_OK) != -1;
}

long FileUtils::getFileSize(string fileName)
{
    struct stat fileInfo;
    int fd = open(fileName.c_str(), O_RDONLY);
    if (fd != -1) {
        if (fstat(fd, &fileInfo) != -1) {
            return fileInfo.st_size;
        }
        close(fd);
    }

    throw runtime_error("Error while getting the file size");
}