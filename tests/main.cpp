// main.cpp
#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <numeric>
#include <vector>

std::mutex cout_mutex;

// 计算向量和的函数
int vector_sum(const std::vector<int>& vec) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟耗时操作
    return std::accumulate(vec.begin(), vec.end(), 0);
}

// 简单的无返回值任务
void print_message(const std::string& msg) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::lock_guard<std::mutex> lock(cout_mutex); // 保护cout
    std::cout << "Message: " << msg << " (thread " 
              << std::this_thread::get_id() << ")" << std::endl;
}

int main() {
    ThreadPool pool(4);
    
    // 测试无返回值任务
    pool.enqueue([] { print_message("Hello"); });
    pool.enqueue([] { print_message("World"); });
    
    // 测试有返回值任务
    std::vector<int> data1 = {1, 2, 3, 4, 5};
    std::vector<int> data2 = {10, 20, 30, 40, 50};
    
    auto future1 = pool.enqueueWithResult(vector_sum, data1);
    auto future2 = pool.enqueueWithResult(vector_sum, data2);
    
    // 做一些其他工作...
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 获取结果
    try {
        int result1 = future1.get();
        int result2 = future2.get();
        
        std::cout << "Sum of data1: " << result1 << std::endl;
        std::cout << "Sum of data2: " << result2 << std::endl;
        std::cout << "Total sum: " << result1 + result2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    // 演示异常传播
    auto future3 = pool.enqueueWithResult([] {
        throw std::runtime_error("Task failed intentionally");
        return 42;
    });
    
    try {
        auto result = future3.get();
    } catch (const std::exception& e) {
        std::cout << "Caught exception from task: " << e.what() << std::endl;
    }
    
    // 等待所有任务完成
    while (pool.queueSize() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}