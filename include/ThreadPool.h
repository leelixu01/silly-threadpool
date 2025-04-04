#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool {
public:
    using Task = std::function<void()>;
    using Priority = unsigned;

    explicit ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    // 添加任务到队列
    template<typename F>
    void enqueue(F&& task);

    // 添加有返回值的任务
    template<typename F, typename... Args>
    auto enqueueWithResult(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>;
    
    // 带优先级的任务提交
    template<typename F>
    void enqueueWithPriority(F&& task, Priority priority);

    // 批量提交任务
    template<typename F>
    void enqueueBatch(std::vector<F>&& tasks);

    // 异步执行函数(类似std::async但使用线程池)
    template<typename F, typename... Args>
    auto async(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>;

    // 等待所有任务完成
    void waitAll();

    size_t getTasksCompleted() const;
    size_t getTasksQueued() const;
    size_t getThreadCount() const;


    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    struct PriorityTask {
        Task task;
        Priority priority;
        
        bool operator<(const PriorityTask& other) const {
            return priority < other.priority; // 更高优先级的值更小
        }
    };

    void worker();

    std::vector<std::thread> workers_;
    std::priority_queue<PriorityTask> tasks_;

    // 同步原语
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

    // 原子计数器
    std::atomic<size_t> tasks_completed_{0};
    std::atomic<size_t> active_tasks_{0};
};

template<typename F>
void ThreadPool::enqueueWithPriority(F&& task, Priority priority) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.push({std::forward<F>(task), priority});
    }
    cv_.notify_one();
}

template<typename F>
void ThreadPool::enqueue(F&& task) {
    enqueueWithPriority(std::forward<F>(task), 0);
}

template<typename F, typename... Args>
auto ThreadPool::enqueueWithResult(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>> {
    
    using return_type = typename std::invoke_result_t<F, Args...>;
    
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    
    std::future<return_type> res = task->get_future();
    enqueue([task]() { (*task)(); });
    return res;
}

// 非const版本，支持移动语义
template<typename F>
void ThreadPool::enqueueBatch(std::vector<F>&& tasks) {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    for (auto& task : tasks) {
        tasks_.push(PriorityTask{
            Task(std::forward<F>(task)),  // 完美转发
            0
        });
    }
    cv_.notify_all();
}
template<typename F, typename... Args>
auto ThreadPool::async(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>> {
    return enqueueWithResult(std::forward<F>(f), std::forward<Args>(args)...);
}

inline size_t ThreadPool::getTasksCompleted() const {
    return tasks_completed_.load(std::memory_order_relaxed);
}


inline size_t ThreadPool::getTasksQueued() const {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

inline size_t ThreadPool::getThreadCount() const {
    return workers_.size();
}

#endif 