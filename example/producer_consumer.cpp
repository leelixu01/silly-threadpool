// producer_consumer.cpp
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::queue<int> data_queue;
std::mutex mtx;
std::condition_variable cv;
constexpr int MAX_ITEMS = 10;

void producer() {
    for (int i = 1; i <= MAX_ITEMS; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        cv.notify_one();
    }
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !data_queue.empty(); });
        
        int val = data_queue.front();
        data_queue.pop();
        lock.unlock();
        
        std::cout << "Consumed: " << val << " by thread " 
                  << std::this_thread::get_id() << std::endl;
        
        if (val == MAX_ITEMS) break;
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons1(consumer);
    std::thread cons2(consumer);
    
    prod.join();
    cons1.join();
    cons2.join();
    
    return 0;
}