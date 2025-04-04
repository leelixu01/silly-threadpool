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

            task = std::move(tasks_.top().task);
            tasks_.pop();
            active_tasks_++;
        }

        try {
            task();
        } catch (...) {
            // 捕获所有异常防止线程退出
            std::lock_guard<std::mutex> lock(queue_mutex_);
            std::cerr << "Exception in worker thread\n";
        }

        tasks_completed_++;
        active_tasks_--;
        cv_.notify_all(); // 通知waitAll可能等待的线程
    }
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    cv_.wait(lock, [this] { return tasks_.empty() && active_tasks_ == 0; });
}
 
