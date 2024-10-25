#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>
#include <cstring>
#include <mutex>

// Default values
const int DEFAULT_THREADS = 2;
const int DEFAULT_SLOP = 10;
const int DEFAULT_WORK_TIME = 10;
const int DEFAULT_ITERATIONS = 100;
const bool DEFAULT_CPU_BOUND = false;
const bool DEFAULT_LOGGING = false;

struct ThreadData {
    int threadIndex;
    int sloppiness;
    int workTime;
    int workIterations;
    bool cpuBound;
    std::atomic<int> *globalCounter;
    std::vector<int> *localBuckets;
    std::mutex *counterMutex;
};

void *threadFunction(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    int localCount = 0;

    for (int i = 0; i < data->workIterations; ++i) {
        if (data->cpuBound) {
            // CPU-bound work: busy loop for workTime
            for (int j = 0; j < data->workTime * 1e6; ++j);
        } else {
            int sleepTime = data->workTime * 500 + (rand() % data->workTime * 500);
            usleep(sleepTime);
        }

        localCount++;
        
        if (localCount == data->sloppiness) {
            std::lock_guard<std::mutex> lock(*data->counterMutex);
            *(data->globalCounter) += localCount;
            localCount = 0;
        }
    }
    
    if (localCount > 0) {
        std::lock_guard<std::mutex> lock(*data->counterMutex);
        *(data->globalCounter) += localCount;
    }
    
    return nullptr;
}

int main(int argc, char *argv[]) {
    int nThreads = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_THREADS;
    int sloppiness = (argc > 2) ? std::stoi(argv[2]) : DEFAULT_SLOP;
    int workTime = (argc > 3) ? std::stoi(argv[3]) : DEFAULT_WORK_TIME;
    int workIterations = (argc > 4) ? std::stoi(argv[4]) : DEFAULT_ITERATIONS;
    bool cpuBound = (argc > 5) ? (strcmp(argv[5], "true") == 0) : DEFAULT_CPU_BOUND;
    bool doLogging = (argc > 6) ? (strcmp(argv[6], "true") == 0) : DEFAULT_LOGGING;

    std::cout << "Settings:\n";
    std::cout << "Threads: " << nThreads << ", Sloppiness: " << sloppiness
              << ", Work Time: " << workTime << " ms, Iterations: " << workIterations
              << ", CPU Bound: " << (cpuBound ? "true" : "false")
              << ", Logging: " << (doLogging ? "true" : "false") << "\n";

    std::atomic<int> globalCounter(0);
    std::mutex counterMutex;
    std::vector<int> localBuckets(nThreads, 0);

    std::vector<pthread_t> threads(nThreads);
    std::vector<ThreadData> threadData(nThreads);

    for (int i = 0; i < nThreads; ++i) {
        threadData[i] = { i, sloppiness, workTime, workIterations, cpuBound, &globalCounter, &localBuckets, &counterMutex };
        pthread_create(&threads[i], nullptr, threadFunction, &threadData[i]);
    }

    if (doLogging) {
        int loggingInterval = workTime * workIterations / 10;
        for (int i = 0; i < 10; ++i) {
            usleep(loggingInterval * 1000);
            std::cout << "[Logging] Global Counter: " << globalCounter.load() << "\n";
        }
    }

    for (int i = 0; i < nThreads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    std::cout << "Final Global Counter: " << globalCounter << "\n";
    return 0;
}
