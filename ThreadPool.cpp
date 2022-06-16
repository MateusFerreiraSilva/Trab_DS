#include "ThreadPool.h"

void ThreadPool::start() {
    cout << "Starting... thread pool\n";
    const uint32_t num_threads = thread::hardware_concurrency(); // Max # of threads the system supports
    // const uint32_t num_threads = 5; // Max # of threads the system supports
    cout << "Num threads: " << num_threads << endl;
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
        cout << "Stopping Threads" << endl;
        {
            unique_lock<mutex> lock(queue_mutex);
            should_terminate = true;
        }
        mutex_condition.notify_all();
        for (std::thread& active_thread : threads) {
            active_thread.join();
        }
        threads.clear();
        cout << "Success Stopping Threads" << endl;
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
        while (true) {
            hasJob = false;
            void (*job)(int);
            int arg;
            {
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
            }
            if (hasJob) {
                cout << "#####" << this_thread::get_id() << " Gonna do the job#####" << endl;
                (*job)(arg); // do job
                cout << "++++++" << this_thread::get_id() << " Has Done the job ++++++" << endl;
            }
        }
}

// compile g++ threadPool.cpp -lpthread

// int main() {
//     cout << "Hello from the main\n";

//     auto threadPool = new ThreadPool();
//     threadPool->start();
//     cout << "Waiting threads to start...\n";
//     this_thread::sleep_for(chrono::seconds(3));
    
//     auto totalJobs = 1000;
//     for (int i = 0; i < totalJobs; i++) {
//         auto job = [](thread::id threadId) { cout << "hello from thread: " << threadId << endl; };
//         threadPool->queueJob(job);
//     }

//     while(1) {
//         this_thread::sleep_for(chrono::seconds(1));
//     }

//     cout << "Goodbye from the main\n";
//     return 0;
// }