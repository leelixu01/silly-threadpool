#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>
#include <sstream>
#include <iomanip>

// 辅助函数：带高精度时间戳的输出（精确到毫秒）
void print_with_time(const std::string& message) {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    
    // 转换为time_t（秒级精度）
    auto now_time = std::chrono::system_clock::to_time_t(now);
    
    // 转换为tm结构体（本地时间）
    std::tm tm = *std::localtime(&now_time);
    
    // 获取毫秒部分
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // 使用stringstream格式化输出
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") 
        << '.' << std::setfill('0') << std::setw(3) << ms.count()
        << " | " << message;
    
    // 输出到标准输出（加锁保证线程安全）
    static std::mutex cout_mutex;
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << oss.str() << std::endl;
}
int main() {
    std::cout << "=== Testing ThreadPool ===" << std::endl;
    
    // 1. 创建线程池
    ThreadPool pool(4);
    std::cout << "Created ThreadPool with " << pool.getThreadCount() << " threads\n";
    
    // // 2. 测试优先级任务
    std::cout << "\n--- Testing Priority Tasks ---\n";
    pool.enqueueWithPriority([] {
        print_with_time("Low priority task (1) started");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        print_with_time("Low priority task (1) finished");
    }, 1);
    
    pool.enqueueWithPriority([] {
        print_with_time("High priority task (3) started");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        print_with_time("High priority task (3) finished");
    }, 3);
    
    pool.enqueueWithPriority([] {
        print_with_time("Medium priority task (2) started");
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
        print_with_time("Medium priority task (2) finished");
    }, 2);
    
    // 3. 测试async接口
    std::cout << "\n--- Testing Async Interface ---\n";
    auto future1 = pool.async([](int a, int b) {
        print_with_time("Async task started");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return a + b;
    }, 10, 20);
    
    std::cout << "Main thread can do other work while async task runs...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Async task result: " << future1.get() << std::endl;
    
    // 4. 测试批量任务提交
    std::cout << "\n--- Testing Batch Task Submission ---\n";
    std::vector<std::function<void()>> tasks;
    std::atomic<int> completed_count{0};
    std::mutex cout_mutex;  // 在外部创建互斥量
    
    for (int i = 0; i < 10; ++i) {
        tasks.emplace_back([i, &completed_count, &cout_mutex] {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Batch task " << i << " completed\n";

            completed_count++;
        });
    }
    
    pool.enqueueBatch(std::move(tasks));
    std::cout << "Submitted 10 batch tasks\n";
    
    // 5. 测试waitAll功能
    std::cout << "\n--- Testing waitAll Functionality ---\n";
    std::atomic<bool> all_done{false};
    
    pool.enqueue([&all_done] {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        all_done = true;
    });
    
    std::thread monitor([&pool, &all_done] {
        while (!all_done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Monitor: " << pool.getTasksCompleted() << " tasks completed, "
                      << pool.getTasksQueued() << " in queue\n";
        }
    });
    
    std::cout << "Waiting for all tasks to complete...\n";
    pool.waitAll();
    all_done = true;
    monitor.join();
    
    std::cout << "\nFinal Stats:\n";
    std::cout << "  Total tasks completed: " << pool.getTasksCompleted() << "\n";
    std::cout << "  Current queue size: " << pool.getTasksQueued() << "\n";
    std::cout << "  Batch tasks completed: " << completed_count << "/10\n";
    
    // 6. 测试异常处理
    std::cout << "\n--- Testing Exception Handling ---\n";
    try {
        auto future = pool.async([] {
            throw std::runtime_error("Intentional exception from task");
            return 42;
        });
        
        future.get();
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== All Tests Completed ===" << std::endl;
    return 0;
}