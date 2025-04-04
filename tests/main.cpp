// main.cpp
#include "ThreadPool.h"
#include <iostream>
#include <chrono>

int main() {
    ThreadPool pool(4);
    
    // 添加10个任务到线程池
    for (int i = 0; i < 10; ++i) {
        pool.enqueue([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Task " << i << " executed by thread " 
                      << std::this_thread::get_id() << std::endl;
        });
    }
    
    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}