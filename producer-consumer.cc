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


// Function to calculate mode
std::vector<int> calculate_mode(const std::vector<int>& data, int top_n = 3) {
    std::map<int, int> frequency_map;
    for (int val : data) {
        frequency_map[val]++;
    }

    std::vector<std::pair<int, int>> frequency_pairs(frequency_map.begin(), frequency_map.end());
    std::sort(frequency_pairs.begin(), frequency_pairs.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });

    std::vector<int> modes;
    for (size_t i = 0; i < std::min(static_cast<size_t>(top_n), frequency_pairs.size()); ++i) {
        modes.push_back(frequency_pairs[i].first);
    }

    return modes;
}

// Template for readers with processing policies
template<typename Processor>
void reader(int reader_id, std::vector<int>& history, Processor process_func) {
    for (int i = 0; i < MAX_ITERATIONS && !termination_flag; ++i) {
        int value;
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            buffer_not_empty.wait(lock, [] { return !buffer.empty() || termination_flag; });
            
            if (termination_flag) break;
            
            value = buffer.front();
            buffer.pop();
            history.push_back(value);
            
            {
                std::lock_guard<std::mutex> cout_lock(cout_mutex);
                std::cout << "[Reader " << reader_id << "] Consumed: " << value << std::endl;
            }
        }
        
        buffer_not_full.notify_one();
        
        // Specific reader processing
        process_func(history, value);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(READER_DELAY_MS));
    }
    
    std::lock_guard<std::mutex> cout_lock(cout_mutex);
    std::cout << "Reader " << reader_id << " completed after " << MAX_ITERATIONS << " iterations." << std::endl;
}

int main() {
    // Setup signal handlers
    signal(SIGINT, handle_termination);
    signal(SIGTERM, handle_termination);

    // Initialize base vector
    initialize_vector();

    // Start threads
    std::thread writer_thread(writer);
    
    // Reader 1: Mode of the last 3 values
    std::thread reader1_thread([&]() {
        reader(1, reader1_history, [](const auto& history, int value) {
            if (history.size() >= 3) {
                auto last_three = std::vector<int>(history.end() - 3, history.end());
                auto modes = calculate_mode(last_three);
                
                std::lock_guard<std::mutex> cout_lock(cout_mutex);
                std::cout << "Reader 1 (Mode): ";
                for (int m : modes) std::cout << m << " ";
                std::cout << std::endl;
            }
        });
    });
    
    // Reader 2: Statistics of all values
    std::thread reader2_thread([&]() {
        reader(2, reader2_history, [](const auto& history, int value) {
            if (!history.empty()) {
                auto stats = calculate_statistics(history);
                
                std::lock_guard<std::mutex> cout_lock(cout_mutex);
                std::cout << std::fixed << std::setprecision(4);
                std::cout << "Reader 2 (Stats): Mean=" << stats.mean 
                          << ", StdDev=" << stats.std_dev 
                          << ", Variance=" << stats.variance << std::endl;
            }
        });
    });
    
    // Reader 3: Cumulative sum
    std::thread reader3_thread([&]() {
        reader(3, reader3_history, [](const auto& history, int value) {
            int sum = std::accumulate(history.begin(), history.end(), 0);
            
            std::lock_guard<std::mutex> cout_lock(cout_mutex);
            std::cout << "Reader 3 (Sum): " << sum << " (Count: " << history.size() 
                      << ", Current: " << value << ")" << std::endl;
        });
    });

    // Wait for threads to finish
    writer_thread.join();
    reader1_thread.join();
    reader2_thread.join();
    reader3_thread.join();

    // Final summary
    std::cout << "\nFinal Summary:" << std::endl;
    std::cout << "  - Reader 1 processed " << reader1_history.size() << " items" << std::endl;
    std::cout << "  - Reader 2 processed " << reader2_history.size() << " items" << std::endl;
    std::cout << "  - Reader 3 processed " << reader3_history.size() << " items" << std::endl;
    std::cout << "  - Remaining in buffer: " << buffer.size() << std::endl;

    return 0;
}
