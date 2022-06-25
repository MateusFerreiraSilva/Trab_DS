#include <bits/stdc++.h>
using namespace std;

#ifndef THREAD_POOL
#define THREAD_POOL

class ThreadPool  {
    private:
        int numThreads;
        inline static const int threadRecvBufferSize = 1000 * 1024 * 2; // 2 MB
        inline static const int threadSendBufferSize = 32767; // max send size, default limit of send system call
        map<thread::id, char*> threadRecvBuffer;
        map<thread::id, char*> threadSendBuffer;
        bool shouldTerminate = false;        // Tells threads to stop looking for jobs
        mutex queueMutex;                   // Prevents data races to the job queue
        vector<thread> threads;
        queue<pair<void(*)(int), int>> jobs;

        void threadLoop();
        void allocateResources();
        void threadExit();
    public:
        ThreadPool();

        void start();
        void queueJob(void(*)(int), int arg);
        void stop();
        bool busy();

        char* getRecvBuffer();
        int getRecvBufferSize();
        char* getSendBuffer();
        int getSendBufferSize();
};

#endif