# README: realtime

## Brief

Facilities to support real-time and low-latency tasks

## OS configuration

The technical note [Configuring Real-time Ubuntu](./docs/realtime_ubuntu.md) describes how to 
configure the target compute platform for low-latency applications.

## Implementation guidelines for real-time and low-latency applications

![A real-time application](docs/media/rt-application.png)

The [`Thread`](include/grape/realtime/thread.h) class enables writing applications such that the 
real-time task is delegated to a separate thread with its own CPU affinity and priority. The main 
thread continues processing lower priority tasks such as I/O, user input handling, graphical 
updates, etc (frameworks such as Qt require event handling loop to run in the main thread). 

Delegate each real-time task to it's own `Thread` instance as follows:

- Configure the RT task in the `setup()` function:
  - Allocate all required memory
  - Assign a scheduling policy using `setSchedule()`
  - Assign a CPU core using `setCpuAffinity()`
- Implement the time-critical step update in `process()`. Additionally, this path should:
  - Avoid locks. Use lock-free atomic variables. Where unavoidable, use priority-inversion safe 
    [`Mutex`](include/grape/realtime/mutex.h)
  - Avoid OS system calls 
  - Avoid third-party code whose worst-case execution time is unknown
  - Avoid memory allocation/deallocation. 
  - Avoid I/O (including console output). Use `MPSCQueue` or lock-free ring-buffers as the 
    intermediary to transfer data both ways from the task
  - Avoid algorithms > O(1) in complexity
  - Only use monotonic clock for timing
- Implement task cleanup before exiting in `teardown()`
- Consider disabling memory swapping _for the process address space_ with `lockMemory()`, 
  preferably at the top of the application.

See [thread_example.cpp](examples/thread_example.cpp) that illustrates these principles.

Facilities such as a watchdog to monitor the health of the real-time task thread can be implemented 
in a similar way.  

#### A note on exceptions

There is no cost to exceptions until they are thrown. Therefore, use them to signal unrecoverable 
errors in the task thread. The task thread will terminate on exception but the main thread will not.
Use `grape::Exception` to capture diagnostic information during exit.

## TODO

- [ ] Implement single producer multi-consumer queue using heap and shared memory
- [ ] Refactor multi-producer single-consumer queue
  - Compare `MPSCQueue` against `FIFOBuffer`. Remove one. 
  - Rename it for clarity
- [ ] Refactor thread class out of realtime and put it in 'grape'
  - Insert logging to capture timer overruns in the loop
- [ ] Application note on how to design the architecture of a realtime loop 
  - Passing data across RT/non-RT boundary using shm-based ring buffers
  - Notion of ticks and timestamps

## References

- [Real-time Ubuntu](https://ubuntu.com/real-time)  
- [Realtime Linux Project](https://wiki.linuxfoundation.org/realtime/start)
- [Real-time programming with the C++ standard library](https://youtu.be/Tof5pRedskI)
- What is low latency C++: [Part 1](https://youtu.be/EzmNeAhWqVs), [Part 2](https://youtu.be/5uIsadq-nyk)
