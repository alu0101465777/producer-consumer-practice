#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <atomic>
#include <csignal>
#include <numeric>
#include <iomanip>

// Constants configuration
constexpr int N = 100;
constexpr int BUFFER_SIZE = 10;
constexpr int MAX_ITERATIONS = 20;
constexpr int WRITER_DELAY_MS = 1000;
constexpr int READER_DELAY_MS = 1500;

// Shared data structures
std::vector<int> vector_base(N);
std::queue<int> buffer;

// Synchronization mechanisms
std::mutex mtx;
std::mutex cout_mutex;
std::condition_variable buffer_not_full;
std::condition_variable buffer_not_empty;
std::atomic<bool> termination_flag{false};

// Reader history
std::vector<int> reader1_history;
std::vector<int> reader2_history;
std::vector<int> reader3_history;


