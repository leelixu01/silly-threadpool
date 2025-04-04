#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <cstddef>
#include <utility>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    using Task = std::function<void()>;

    explicit ThreadPool(std::size_t num_threads);
    ~ThreadPool();

    // 添加任务到队列
    template<typename F>
    void enqueue(F&& task);

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void worker();

    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;

    // 同步原语
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

};

template<typename F>
void ThreadPool::enqueue(F&& task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.emplace(std::forward<F>(task));
    }
    // 通知进程有任务可以获取了
    cv_.notify_one();
}


#endif 