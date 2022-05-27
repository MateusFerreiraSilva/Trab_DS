#include "threadPool.h"

// void ThreadPool::ThreadLoop() {
//     while (true) {
//         function<void()> job;
//         {
//             unique_lock<mutex> lock(queue_mutex);
//             mutex_condition.wait(lock, [this] {
//                 return !jobs.empty() || should_terminate;
//             });
//             if (should_terminate) {
//                 return;
//             }
//             job = jobs.front();
//             jobs.pop();
//         }
//         job();
//     }
// }

// void ThreadPool::Start() {
//     cout << "Starting... thread pool\n";
//     // const uint32_t num_threads = thread::hardware_concurrency(); // Max # of threads the system supports
//     const uint32_t num_threads = 3; // Max # of threads the system supports
//     threads.resize(num_threads);
//     for (uint32_t i = 0; i < num_threads; i++) {
//         threads.at(i) = thread(ThreadLoop);
//     }
// }

// void ThreadPool::QueueJob(const function<void()>& job) {
//     {
//         unique_lock<mutex> lock(queue_mutex);
//         jobs.push(job);
//     }
//     mutex_condition.notify_one();
// }

// // weird
// bool ThreadPool::busy() {
//     bool poolbusy;
//     {
//         unique_lock<mutex> lock(queue_mutex);
//         poolbusy = jobs.empty();
//     }
//     return poolbusy;
// }

// void ThreadPool::Stop() {
//     {
//         unique_lock<mutex> lock(queue_mutex);
//         should_terminate = true;
//     }
//     mutex_condition.notify_all();
//     for (std::thread& active_thread : threads) {
//         active_thread.join();
//     }
//     threads.clear();
// }

// compile g++ threadPool.cpp -lpthread

int main() {
    cout << "Hello from the main\n";

    auto threadPool = new ThreadPool();
    threadPool->Start();
    cout << "Waiting threads to start...\n";
    this_thread::sleep_for(chrono::seconds(3));
    
    auto totalJobs = 1000;
    for (int i = 0; i < totalJobs; i++) {
        auto job = [](thread::id threadId) { cout << "hello from thread: " << threadId << endl; };
        threadPool->QueueJob(job);
    }

    while(1) {
        this_thread::sleep_for(chrono::seconds(1));
    }

    cout << "Goodbye from the main\n";
    return 0;
}