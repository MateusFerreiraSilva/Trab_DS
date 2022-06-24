#ifndef THREAD_POOL
#define THREAD_POOL

#include <bits/stdc++.h>

using namespace std;

class ThreadPool  {
    private:
        int num_threads;
        bool should_terminate = false;        // Tells threads to stop looking for jobs
        mutex queue_mutex;                   // Prevents data races to the job queue
        condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
        vector<thread> threads;
        queue<pair<void(*)(int), int>> jobs;

        void threadLoop();
    public:
        void start();
        void queueJob(void(*)(int), int arg);
        void stop();
        bool busy();
};

#endif