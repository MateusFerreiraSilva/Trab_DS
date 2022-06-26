#include "ThreadPool.h"

ThreadPool::ThreadPool() {
    // 200 best
    numThreads = 200; // Max # of threads the system supports
    // numThreads = thread::hardware_concurrency();
    printf("Num threads: %d\n", numThreads);
}

void ThreadPool::start() {
    printf("Starting... thread pool\n");
    threads.resize(numThreads);
    for (int i = 0; i < numThreads; i++) {
        threads.at(i) = thread(&ThreadPool::threadLoop, this);
    }
}

void ThreadPool::stop() {
    printf("Stopping Threads");

    shouldTerminate = true;

    for (thread& active_thread : threads) {
        active_thread.join();
    }

    threads.clear();

    printf("Success Stopping Threads\n");
}

void ThreadPool::allocateThreadResources() {
    threadRecvBuffer[this_thread::get_id()] = (char*) malloc(threadRecvBufferSize * sizeof(char));
    threadSendBuffer[this_thread::get_id()] = (char*) malloc(threadSendBufferSize * sizeof(char));
}

void ThreadPool::deallocateThreadResources() {
    // printf("Realising threads...\n");

    if (threadRecvBuffer[this_thread::get_id()] != NULL) {
        free(threadRecvBuffer[this_thread::get_id()]);
    }
    if (threadSendBuffer[this_thread::get_id()] != NULL)
    {
        free(threadSendBuffer[this_thread::get_id()]);
    }
}

void ThreadPool::queueJob(void (*job)(int), int arg) {
    queueMutex.lock();
    
    {
        jobs.push(make_pair(job, arg));
    }

    queueMutex.unlock();
}

void ThreadPool::threadLoop() {
    // cout << "-------Thread " << this_thread::get_id() << " has started-------\n" << endl;
    
    allocateThreadResources();
    if (threadRecvBuffer[this_thread::get_id()] == NULL || threadSendBuffer[this_thread::get_id()] == NULL)
        return;

    bool hasJob;
    void (*job)(int);
    int arg;

    while (true) {
        hasJob = false;

        queueMutex.lock();
        
        {
            if (shouldTerminate) {
                queueMutex.unlock();
                deallocateThreadResources();
                return;
            }

            if (!jobs.empty()) {
                pair<void(*)(int), int> newJob = jobs.front();
                job = newJob.first;
                arg = newJob.second;
                jobs.pop();
                hasJob = true;
            }
        }

        queueMutex.unlock();
        
        if (hasJob) {
            // cout << "##### Thread " << this_thread::get_id() << " Gonna do the job #####" << endl;
            (*job)(arg); // do job
            // cout << "++++++ Thread " << this_thread::get_id() << " Has Done the job ++++++" << endl;
        }
    }
}

char* ThreadPool::getRecvBuffer() {
    return threadRecvBuffer[this_thread::get_id()];
}

int ThreadPool::getRecvBufferSize() {
    return threadRecvBufferSize;
}

char* ThreadPool::getSendBuffer() {
    return threadSendBuffer[this_thread::get_id()];
}

int ThreadPool::getSendBufferSize() {
    return threadSendBufferSize;
}

bool ThreadPool::shouldStop() {
    return shouldTerminate;
}