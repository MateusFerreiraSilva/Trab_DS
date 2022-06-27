#include <stdio.h>
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <thread>
#include <mutex>
using namespace std;

#ifndef THREAD_POOL
#define THREAD_POOL

class ThreadPool  {
    private:
        const int numThreads = 200; // 200 best
        // const int numThreads = thread::hardware_concurrency(); // Max # of threads the system supports
        const int threadRecvBufferSize = 1024 * 1024 * 2; // 2 MB
        const int threadSendBufferSize = 32767; // max send size (32 KB aprox), default limit of send system call
        map<thread::id, char*> threadRecvBuffer;
        map<thread::id, char*> threadSendBuffer;
        bool shouldTerminate = false;        // Tells threads to stop looking for jobs
        mutex queueMutex;                   // Prevents data races to the job queue
        vector<thread> threads;
        queue<pair<void(*)(int), int>> jobs;

        void allocateThreadResources();
        void deallocateThreadResources();
        void threadLoop();
    public:
        ThreadPool();

        void start();
        void stop();
        void queueJob(void(*)(int), int arg);

        char* getRecvBuffer();
        int getRecvBufferSize();
        char* getSendBuffer();
        int getSendBufferSize();
        bool shouldStop();
};

#endif