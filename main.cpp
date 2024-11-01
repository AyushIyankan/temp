#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <chrono>         // For high-resolution clock and std::chrono::duration
#include <thread>         // For std::this_thread::sleep_for
#include <cmath>          // For mathematical operations (sin, cos)
#include <vector>         // For storing threads
#include <sched.h>        // For setting thread affinity
#include <unistd.h>
#include <sys/wait.h>  // For waitpid()

// Function to simulate CPU-bound work for each thread
void do_work(int threadIdx) {
    double result = 0.0;
    for (int i = 0; i < 1e7; ++i) {
        result += std::sin(i) * std::cos(i);  // Simulate CPU work
    }
    std::cout << "Thread " << threadIdx << " completed work.\n";
}

// Function to get thread affinity (returns logical core number)
int32_t get_thread_affinity(uint32_t threadIdx) {
    // Simple round-robin pinning to logical cores based on threadIdx
    return threadIdx % 4;  // Assume 4 logical cores for simplicity
}

// Function to set thread affinity
void set_affinity(int threadIdx) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(get_thread_affinity(threadIdx), &cpuset);  // Use get_thread_affinity to set the CPU

    pthread_t thread = pthread_self();
    int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        std::cerr << "Error setting affinity for thread " << threadIdx << std::endl;
    }
}



void start_monitoring_for_thread(uint32_t threadIdx, pid_t tid, pthread_t handle) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: run perf with exec
        std::string perf_file = "perf_output_thread_" + std::to_string(threadIdx) + ".txt";
        std::string tid_str = std::to_string(tid);
        execlp("perf", "perf", "stat", "-p", tid_str.c_str(), "-o", perf_file.c_str(), (char*)NULL);
        exit(0); // Exit child process after exec
    } else if (pid > 0) {
        // Parent process: Store child PID and continue
        // The parent continues to run the thread work
    } else {
        std::cerr << "Error forking process for perf on thread " << threadIdx << std::endl;
    }
}



void stop_monitoring_for_thread([[maybe_unused]] uint32_t threadIdx) {
    // This command will terminate all running instances of perf
    int result = system("killall -q perf");
    if (result == -1) {
        std::cerr << "Error stopping perf for thread " << threadIdx << std::endl;
    }
}


void thread_work(int threadIdx) {
    // Start monitoring
    start_monitoring_for_thread(threadIdx, getpid(), pthread_self());

    // Perform the actual work
    do_work(threadIdx);

    // Stop monitoring
    stop_monitoring_for_thread(threadIdx);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    int num_threads = 3;  // Dummy number of threads for this example
    std::cout << "The number of threads: " << num_threads << std::endl;

    // Measure time before optimization
    auto start = std::chrono::high_resolution_clock::now();

    // Create and run threads without optimization
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(thread_work, i));  // Create threads for the work
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    // Stop time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_taken = end - start;
    std::cout << "Time taken (before optimization): " << time_taken.count() << " seconds\n";

    // Measure time after optimization (applying thread affinity)
    auto start_optimized = std::chrono::high_resolution_clock::now();

    // Clear and run threads with optimization (setting affinity)
    threads.clear();
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread([i]() {
            set_affinity(i);    // Set affinity to logical cores
            thread_work(i);     // Do work after setting affinity
        }));
    }

    // Join all optimized threads
    for (auto& t : threads) {
        t.join();
    }

    // Stop time
    auto end_optimized = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_taken_optimized = end_optimized - start_optimized;
    std::cout << "Time taken (after optimization): " << time_taken_optimized.count() << " seconds\n";

    // Calculate speedup
    double speedup = time_taken.count() / time_taken_optimized.count();
    std::cout << "Speedup: " << speedup << "x\n";

    return 0;
}
