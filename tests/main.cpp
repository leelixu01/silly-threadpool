// main.cpp
#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <vector>

// 无锁计数器演示
class AtomicCounter {
public:
    void increment() {
        counter_.fetch_add(1, std::memory_order_relaxed);
    }
    
    void decrement() {
        counter_.fetch_sub(1, std::memory_order_relaxed);
    }
    
    int get() const {
        return counter_.load(std::memory_order_relaxed);
    }
    
private:
    std::atomic<int> counter_{0};
};

void test_atomic_counter(AtomicCounter& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.increment();
        // 模拟一些工作
        std::this_thread::yield();
    }
}

int main() {
    // 测试原子计数器
    AtomicCounter counter;
    constexpr int iterations = 100000;
    constexpr int thread_count = 4;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(test_atomic_counter, std::ref(counter), iterations);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Expected counter value: " << thread_count * iterations << "\n";
    std::cout << "Actual counter value: " << counter.get() << "\n";
    
    // 测试线程池统计功能
    ThreadPool pool(2);
    
    // 提交100个任务
    for (int i = 0; i < 100; ++i) {
        pool.enqueue([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    
    // 监控任务执行情况
    while (pool.getTasksQueued() > 0 || pool.getTasksCompleted() < 100) {
        std::cout << "Completed: " << pool.getTasksCompleted() 
                  << ", Queued: " << pool.getTasksQueued() 
                  << ", Threads: " << pool.getThreadCount() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "All tasks completed. Total: " << pool.getTasksCompleted() << "\n";
    
    return 0;
}