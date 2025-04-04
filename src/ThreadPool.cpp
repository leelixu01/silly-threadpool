#include "ThreadPool.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] { worker(); });
    }
    std::cout << "ThreadPool created with " << num_threads << " threads" << std::endl;
}

ThreadPool::~ThreadPool() {
    stop_ =  true;
    cv_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            cv_.wait(lock, [this]{
                return stop_ || !tasks_.empty();
            });

            if (stop_ && tasks_.empty()) {
                return;
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        task();
        tasks_completed_.fetch_add(1, std::memory_order_relaxed);
    }
}
 
