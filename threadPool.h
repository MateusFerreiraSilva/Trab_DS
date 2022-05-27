#include <bits/stdc++.h>
using namespace std;

class ThreadPool  {
public:
    void Start() {
        cout << "Starting... thread pool\n";
        // const uint32_t num_threads = thread::hardware_concurrency(); // Max # of threads the system supports
        const uint32_t num_threads = 3; // Max # of threads the system supports
        threads.resize(num_threads);
        for (uint32_t i = 0; i < num_threads; i++) {
            threads.at(i) = thread(&ThreadPool::ThreadLoop, this);
        }
    }

    void QueueJob(const function<void(thread::id)>& job) {
        {
            unique_lock<mutex> lock(queue_mutex);
            jobs.push(job);
        }
        mutex_condition.notify_one();
    }
    
    void Stop() {
        {
            unique_lock<mutex> lock(queue_mutex);
            should_terminate = true;
        }
        mutex_condition.notify_all();
        for (std::thread& active_thread : threads) {
            active_thread.join();
        }
        threads.clear();
    }
    
    bool busy() {
        bool poolbusy;
        {
            unique_lock<mutex> lock(queue_mutex);
            poolbusy = jobs.empty();
        }
        return poolbusy;
    }

private:
    void ThreadLoop() {
        bool hasJob;
        while (true) {
            hasJob = false;
            function<void(thread::id)> job;
            {
                unique_lock<mutex> lock(queue_mutex);
                mutex_condition.wait(lock, [this] {
                    return !jobs.empty() || should_terminate;
                });
                if (should_terminate) {
                    return;
                }
                if (!jobs.empty()) {
                    job = jobs.front();
                    jobs.pop();
                    hasJob = true;
                }
            }
            if (hasJob)
                job(this_thread::get_id());
        }
    }

    bool should_terminate = false;        // Tells threads to stop looking for jobs
    mutex queue_mutex;                   // Prevents data races to the job queue
    condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
    vector<thread> threads;
    queue<function<void(thread::id)>> jobs;
};

