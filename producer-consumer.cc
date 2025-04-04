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

// Initialize base vector with random values
void initialize_vector() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10);
    
    std::lock_guard<std::mutex> lock(mtx);
    std::generate(vector_base.begin(), vector_base.end(), [&]() { return dist(gen); });
}

// Function to handle termination signals
void handle_termination(int signal) {
    std::lock_guard<std::mutex> cout_lock(cout_mutex);
    std::cout << "\nTermination signal received. Exiting..." << std::endl;
    termination_flag = true;
    buffer_not_full.notify_all();
    buffer_not_empty.notify_all();
}

// Improved writer function
void writer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 10);

    for (int i = 0; i < MAX_ITERATIONS && !termination_flag; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WRITER_DELAY_MS));
        
        int value = dist(gen);
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            buffer_not_full.wait(lock, [] { return buffer.size() < BUFFER_SIZE || termination_flag; });
            
            if (termination_flag) break;
            
            buffer.push(value);
            
            {
                std::lock_guard<std::mutex> cout_lock(cout_mutex);
                std::cout << "[Writer] Produced: " << value << " (Buffer size: " << buffer.size() << ")" << std::endl;
            }
        }
        
        buffer_not_empty.notify_one();
    }
    
    std::lock_guard<std::mutex> cout_lock(cout_mutex);
    std::cout << "Writer completed after " << MAX_ITERATIONS << " iterations." << std::endl;
}

// Function to calculate statistics
struct Statistics {
    double mean;
    double variance;
    double std_dev;
};

Statistics calculate_statistics(const std::vector<int>& data) {
    Statistics stats{0.0, 0.0, 0.0};
    if (data.empty()) return stats;

    stats.mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    
    for (int val : data) {
        stats.variance += std::pow(val - stats.mean, 2);
    }
    stats.variance /= data.size();
    stats.std_dev = std::sqrt(stats.variance);
    
    return stats;
}

