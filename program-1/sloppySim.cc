// Copyright 2024 Ian Lecker

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>

// Using statements
using std::cout;
using std::vector;

// Defaults
const int DEFAULT_THREADS = 2;
const int DEFAULT_SLOP = 10;
const int DEFAULT_WORK_TIME = 10;
const int DEFAULT_ITERATIONS = 100;
const bool DEFAULT_CPU_BOUND = false;
const bool DEFAULT_LOGGING = false;

// Holds data shared between threads
struct SharedData {
    int globalCounter;
    pthread_mutex_t counterMutex;
    vector<int> localBuckets;

    SharedData(int nThreads) : globalCounter(0), localBuckets(nThreads, 0) {
        pthread_mutex_init(&counterMutex, nullptr); // initializes mutex at address of counterMutex
    }

    ~SharedData() { // Destructor to avoid a memory leak
        pthread_mutex_destroy(&counterMutex);
    }
};

// Structure for thread-specific data
struct ThreadData {
    int threadIndex;
    int sloppiness;
    int workTime;
    int workIterations;
    bool cpuBound;
    SharedData* shared;

    ThreadData(int index, int slop, int time, int iterations, bool cpu, SharedData* sh)
        : threadIndex(index), sloppiness(slop), workTime(time),
          workIterations(iterations), cpuBound(cpu), shared(sh) {}
};

void* threadFunction(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg); // ensures the arg is in the ThreadData format

    unsigned int seed = time(nullptr) + data->threadIndex; // unique RNG seed per thread

    // Work simulation loop
    for (int i = 0; i < data->workIterations; ++i) {
        if (data->cpuBound) {
            // CPU-bound work
            for (int j = 0; j < data->workTime * 1000000; ++j) {}
        } else {
            // I/O-bound work with random time between 0.5 and 1.5 times workTime
            int sleepTime = (data->workTime * (500 + (rand_r(&seed) % 1000))) / 1000; // This is in milliseconds
            usleep(sleepTime * 1000); // Convert to microseconds
        }

        data->shared->localBuckets[data->threadIndex]++; // once a piece of "work" is done, increment the thread's local bucket

        // Update global counter once the number of work units is greater than the sloppiness
        if (data->shared->localBuckets[data->threadIndex] >= data->sloppiness) {
            pthread_mutex_lock(&data->shared->counterMutex);
            data->shared->globalCounter += data->shared->localBuckets[data->threadIndex]; // add local bucket to global counter
            data->shared->localBuckets[data->threadIndex] = 0; // reset local bucket to 0
            pthread_mutex_unlock(&data->shared->counterMutex);
        }
    }

    // If after the work iterations there are leftover work units but not more than the sloppiness factor, add them to global counter
    if (data->shared->localBuckets[data->threadIndex] > 0) {
        pthread_mutex_lock(&data->shared->counterMutex);
        data->shared->globalCounter += data->shared->localBuckets[data->threadIndex];
        data->shared->localBuckets[data->threadIndex] = 0;
        pthread_mutex_unlock(&data->shared->counterMutex);
    }

    return nullptr;
}

void printBuckets(const SharedData& shared, int nThreads) {
    cout << "[";
    for (int i = 0; i < nThreads; ++i) {
        cout << shared.localBuckets[i];
        if (i < nThreads - 1) cout << ",";
    }
    cout << "] Global: " << shared.globalCounter << std::endl;
}

int main(int argc, char* argv[]) {
    srand(time(nullptr)); // important for RNG

    if (argc < 1 || argc > 7) {
        cout << "Usage: sloppySim <N_Threads> <Sloppiness> <work_time> "
                "<work_iterations> <CPU_BOUND> <Do_Logging>\n";
        return 1;
    }

    // Parse arguments with defaults
    int nThreads = (argc > 1) ? std::stoi(argv[1]) : DEFAULT_THREADS;
    int sloppiness = (argc > 2) ? std::stoi(argv[2]) : DEFAULT_SLOP;
    int workTime = (argc > 3) ? std::stoi(argv[3]) : DEFAULT_WORK_TIME;
    int workIterations = (argc > 4) ? std::stoi(argv[4]) : DEFAULT_ITERATIONS;
    bool cpuBound = (argc > 5) ? (strcmp(argv[5], "true") == 0) : DEFAULT_CPU_BOUND;
    bool doLogging = (argc > 6) ? (strcmp(argv[6], "true") == 0) : DEFAULT_LOGGING;

    // Ensure there is at least one thread
    if (nThreads < 1) {
        cout << "Error: Number of threads must be at least 1." << std::endl;
        return 1;
    }

    // Print out settings
    cout << "Settings:\n"
         << "Threads: " << nThreads
         << "\nSloppiness: " << sloppiness
         << "\nWork Time: " << workTime << "ms"
         << "\nIterations: " << workIterations
         << "\nCPU Bound: " << (cpuBound ? "true" : "false")
         << "\nLogging: " << (doLogging ? "true" : "false") << std::endl;

    // Initialize shared data
    SharedData shared(nThreads);
    vector<pthread_t> threads(nThreads);
    vector<ThreadData> threadData;
    threadData.reserve(nThreads); // Reserve space

    // Create threads
    for (int i = 0; i < nThreads; ++i) {
        ThreadData threadDataObject(i, sloppiness, workTime, workIterations, cpuBound, &shared);
        threadData.push_back(threadDataObject);
        if (pthread_create(&threads[i], nullptr, threadFunction, &threadData[i]) != 0) {
            cout << "Error creating thread " << i << std::endl;
            return 1; // Exit on thread creation failure
        }
    }

    // Logging loop if enabled
    if (doLogging) {
        int loggingInterval = workTime * workIterations / 10;
        for (int i = 0; i < 9; ++i) {
            usleep(loggingInterval * 1000);
            printBuckets(shared, nThreads);
        }
    }

    for (int i = 0; i < nThreads; ++i) { // join threads upon completion
        pthread_join(threads[i], nullptr);
    }

    if (doLogging) {
        printBuckets(shared, nThreads);
    } else {
        cout << "Final Global Counter: " << shared.globalCounter << std::endl;
    }

    return 0;
}
