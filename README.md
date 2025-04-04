# Producer-Consumer System Implementation

## Overview

This C++ program implements a classic producer-consumer pattern with multiple reader threads performing different statistical operations on shared data. The system demonstrates thread synchronization using mutexes and condition variables.

## Features

- **Multi-threaded architecture** with one producer and three consumer threads
- **Thread-safe buffer** with fixed capacity
- **Statistical operations**:
  - Mode calculation (most frequent values)
  - Statistical measures (mean, standard deviation, variance)
  - Cumulative sum
- **Graceful termination** handling via signals
- **Configurable parameters** for buffer size and operation limits

## Requirements

- C++17 compatible compiler (GCC, Clang, MSVC)
- CMake 3.12+ (optional, for build system)
- Linux/macOS/Windows system

## Build Instructions

### Basic build:
```bash
g++ -std=c++17 -pthread producer-consumer.cpp -o producer-consumer