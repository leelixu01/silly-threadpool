#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

std::mutex cout_mutex;  // 保护std::cout

void unsafe_increment(int& counter) {
    for (int i = 0; i < 100000; ++i) {
        counter++;  // 存在竞争条件
    }
}

void safe_increment(int& counter, std::mutex& mtx) {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        counter++;  // 线程安全
    }
}

void print_result(const std::string& name, int expected, int actual) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << name << ": expected=" << expected 
              << ", actual=" << actual << std::endl;
}

int main() {
    // 不安全的计数器
    int unsafe_counter = 0;
    std::thread t1(unsafe_increment, std::ref(unsafe_counter));
    std::thread t2(unsafe_increment, std::ref(unsafe_counter));
    t1.join();
    t2.join();
    print_result("Unsafe counter", 200000, unsafe_counter);
    
    // 安全的计数器
    int safe_counter = 0;
    std::mutex safe_mutex;
    std::thread t3(safe_increment, std::ref(safe_counter), std::ref(safe_mutex));
    std::thread t4(safe_increment, std::ref(safe_counter), std::ref(safe_mutex));
    t3.join();
    t4.join();
    print_result("Safe counter", 200000, safe_counter);
    
    return 0;
}