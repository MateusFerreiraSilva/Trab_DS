#include "ThreadPool.h"

void ThreadPool::start() {
    printf("Starting... thread pool\n");
    num_threads = thread::hardware_concurrency(); // Max # of threads the system supports
    printf("Num threads: %d\n", num_threads);
    threads.resize(num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        threads.at(i) = thread(&ThreadPool::threadLoop, this);
    }
}

void ThreadPool::queueJob(void (*job)(int), int arg) {
    queue_mutex.lock();
    
    {
        jobs.push(make_pair(job, arg));
    }

    queue_mutex.unlock();
}

void ThreadPool::stop() {
    printf("Stopping Threads");

    should_terminate = true;

    for (thread& active_thread : threads) {
        active_thread.join();
    }

    threads.clear();

    printf("Success Stopping Threads\n");
}

bool ThreadPool::busy() {
    bool poolbusy;
    queue_mutex.lock();
    {
        poolbusy = jobs.empty();
    }
    queue_mutex.unlock();

    return poolbusy;
}

void ThreadPool::threadLoop() {
    cout << "-------Thread " << this_thread::get_id() << " has started-------\n" << endl;

    bool hasJob;
    void (*job)(int);
    int arg;

    while (true) {
        hasJob = false;

        queue_mutex.lock();
        
        {
            if (should_terminate) {
                queue_mutex.unlock();
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

        queue_mutex.unlock();
        
        if (hasJob) {
            cout << "##### Thread " << this_thread::get_id() << " Gonna do the job #####" << endl;
            (*job)(arg); // do job
            cout << "++++++ Thread " << this_thread::get_id() << " Has Done the job ++++++" << endl;
        }
    }
}