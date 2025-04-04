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
