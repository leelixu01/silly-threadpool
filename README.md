# 🧵 ThreadPool - A Multithreaded Task Dispatch System

A simple, extensible thread pool implementation in modern C++ (C++11+), designed for lightweight task scheduling and concurrent execution.

## 🚀 Features

- Create a pool of worker threads
- Submit tasks using lambdas or callable objects
- Thread-safe task queue
- Graceful shutdown of all threads
- Minimal dependencies, cross-platform (Linux/macOS/Windows)

## 📁 Project Structure

```
├── include/         # Header files (ThreadPool API)
├── src/             # Implementation (.cpp)
├── example/         # Simple usage examples (a.cpp, b.cpp)
├── tests/           # Unit tests or main.cpp for validation
├── CMakeLists.txt   # Build configuration
└── build/           # (Ignored) Build directory
└── compile_commands.json # (Ignored) Compiler database for IDEs
```

## 🛠️ Build Instructions

### 1. Prerequisites

- C++11 or later
- CMake 3.10+
- POSIX threads (`pthread`) support

### 2. Build (Linux/macOS)

```bash
mkdir build && cd build
cmake ..
make
```

### 3. Run Test

```bash
./tests/main
```

## ✨ Example Usage

Here is an example of how to use the thread pool in your code:

```cpp
#include "ThreadPool.h"
#include <iostream>

int main() {
    // Create a thread pool with 4 threads
    ThreadPool pool(4);

    // Submit a simple task to the thread pool
    pool.enqueue([] {
        std::cout << "Hello from thread pool!" << std::endl;
    });

    // Allow time for the task to execute before exiting
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
```

## 📝 License

MIT License