#include "ThreadPool.h"

void ThreadPool::start() {
    printf("Starting... thread pool\n");
    const uint32_t num_threads = thread::hardware_concurrency(); // Max # of threads the system supports
    printf("Num threads: %d\n", num_threads);
    threads.resize(num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        threads.at(i) = thread(&ThreadPool::threadLoop, this);
    }
}

void ThreadPool::queueJob(void (*job)(int), int arg) {
        {
            unique_lock<mutex> lock(queue_mutex);
            jobs.push(make_pair(job, arg));
        }
        mutex_condition.notify_one();
}

void ThreadPool::stop() {
        printf("Stopping Threads");
        {
            unique_lock<mutex> lock(queue_mutex);
            should_terminate = true;
        }
        mutex_condition.notify_all();
        for (std::thread& active_thread : threads) {
            active_thread.join();
        }
        threads.clear();
        printf("Success Stopping Threads\n");
}

bool ThreadPool::busy() {
        bool poolbusy;
        {
            unique_lock<mutex> lock(queue_mutex);
            poolbusy = jobs.empty();
        }
        return poolbusy;
}

void ThreadPool::threadLoop() {
        cout << "-------Thread " << this_thread::get_id() << " has started-------\n" << endl;

        bool hasJob;
        void (*job)(int);
        int arg;

        while (true) {
            unique_lock<mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] {
                return !jobs.empty() || should_terminate;
            });
            if (should_terminate) {
                return;
            }

            if (!jobs.empty()) {
                job = jobs.front().first;
                arg = jobs.front().second;
                jobs.pop();
                hasJob = true;
            }
            
            if (hasJob) {
                cout << "##### Thread " << this_thread::get_id() << " Gonna do the job #####" << endl;
                (*job)(arg); // do job
                cout << "++++++ Thread " << this_thread::get_id() << " Has Done the job ++++++" << endl;
                hasJob = false;
            }
        }
}