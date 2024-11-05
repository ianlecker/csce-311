# Sloppy Counter Simulator - Ian Lecker
This program implements a sloppy counter simulator.

### Compilation
To compile the program, simply run:

make

This will create the executable sloppySim.

### Usage
./sloppySim <N_Threads> <Sloppiness> <work_time> <work_iterations> <CPU_BOUND> <Do_Logging>

### Arguments
N_Threads: Number of worker threads (default: 2)
Sloppiness: Events before updating global counter (default: 10)
work_time: Average work time in milliseconds (default: 10)
work_iterations: Number of iterations per thread (default: 100)
CPU_BOUND: "true" or "false" - determines work type (default: false)
Do_Logging: "true" or "false" - enables progress logging (default: false)

### Example Usage
Run with 4 threads, sloppiness of 5, 10ms work time, CPU-bound and logging enabled.

./sloppySim 4 5 10 1000 true true

### Run with default values except for number of threads
./sloppySim 8