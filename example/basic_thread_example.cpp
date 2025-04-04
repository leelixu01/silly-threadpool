#include <iostream>
#include <thread>
#include <vector>

void hello(int id) {
    std::cout << "Hello from thread " << id << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    
    // 创建5个线程
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(hello, i);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "All threads completed!" << std::endl;
    return 0;
}