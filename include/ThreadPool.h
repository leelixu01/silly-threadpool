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

    explicit ThreadPool(std::size_t num_threads);
    ~ThreadPool();

    // 添加任务到队列
    template<typename F>
    void enqueue(F&& task);

    // 添加有返回值的任务
    template<typename F, typename... Args>
    auto enqueueWithResult(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>;
    

    size_t getTasksCompleted() const;
    size_t getTasksQueued() const;
    size_t getThreadCount() const;


    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    void worker();

    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;

    // 同步原语
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

    // 原子计数器
    std::atomic<size_t> tasks_completed_{0};
    std::atomic<size_t> tasks_queued_{0};
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

// 模板函数：用于向线程池中添加带返回值的任务
template<typename F, typename... Args>
auto ThreadPool::enqueueWithResult(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>> {
    
    // 推导出函数 f 在传入 args... 参数后所返回的类型
    using return_type = typename std::invoke_result_t<F, Args...>;

    // 创建一个 packaged_task 封装任务，使用 bind 绑定参数，延迟执行
    // 使用 shared_ptr 管理 task 生命周期，避免提前释放
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    // 获取与 task 关联的 future，用于异步获取任务结果
    std::future<return_type> res = task->get_future();

    {
        // 线程安全地向任务队列中添加任务
        std::unique_lock<std::mutex> lock(queue_mutex_);

        // 将任务封装为一个无参数、无返回值的函数对象加入队列
        // 实际上调用的是 shared_ptr 封装的 packaged_task 的 operator()
        tasks_.emplace([task]() { (*task)(); });
    }

    // 通知一个等待的线程有新任务可处理
    cv_.notify_one();

    // 返回 future 给调用者，用于获取异步任务的返回值
    return res;
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