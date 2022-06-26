#include "FileUtils.h"

bool FileUtils::doesFileExist(string fileName) {
    return access(fileName.c_str(), F_OK) != -1;
}

long FileUtils::getFileSize(string fileName) {
    int retries = 0;
    const int maxNumOfRetries = 3;

    struct stat fileInfo;
    int fd = open(fileName.c_str(), O_RDONLY);
    while (fd == -1) {
        retries++;
        if (retries < maxNumOfRetries)
            sleep(1);
        else
            throw runtime_error("Error while opening the file");
    } // succeeded reading a chung of the file

    while (fstat(fd, &fileInfo) == -1) {
        retries++;
        if (retries < maxNumOfRetries)
            sleep(1);
        else
            throw runtime_error("Error while getting the file size");
    } // succeeded getting the file size

    return fileInfo.st_size;
}