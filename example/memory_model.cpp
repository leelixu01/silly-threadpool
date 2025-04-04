#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<bool> x(false), y(false);
std::atomic<int> z(0);

void write_x() {
    x.store(true, std::memory_order_release);
}

void write_y() {
    y.store(true, std::memory_order_release);
}

void read_x_then_y() {
    while (!x.load(std::memory_order_acquire)) {}
    if (y.load(std::memory_order_acquire)) {
        z.fetch_add(1, std::memory_order_relaxed);
    }
}

void read_y_then_x() {
    while (!y.load(std::memory_order_acquire)) {}
    if (x.load(std::memory_order_acquire)) {
        z.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(write_x);
        threads.emplace_back(write_y);
        threads.emplace_back(read_x_then_y);
        threads.emplace_back(read_y_then_x);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "z = " << z << std::endl;
    return 0;
}