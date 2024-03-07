# README: realtime

## Brief

Facilities to support deterministic real-time execution of tasks

## Real-time tasks

### Background

A real-time task has a deterministic upper-bound on response latency. In autonomous systems, this could be in the order of tens of microseconds to ensure correct and safe operation. Achieving such low latency requires both configuration of the platform (OS) and crafting the application following some guidelines.

### Application implementation guidelines 

![A real-time application](docs/media/rt-application.png)

It is easier to write a regular application with a _minimal_ real-time component than the other way around. This is the fundamental idea and the capability that the [`Thread`](include/grape/realtime/thread.h) class provides. It allows an application to be written like any other, but the bit of code that has real-time constraints is delegated to a separate thread. This leaves the main thread and the rest of the application to continue processing lower priority tasks such as I/O, user input handling, graphical updates, etc (Graphics frameworks such as Qt require event handling loop to run in the main thread). Applications implemented this way are easier to develop and reason about, and hence, easier to maintain.

Delegate each real-time task to it's own `Thread` instance as follows:
  - Configure the task in the `setup()` function:
    - Allocate all the required memory
    - Assign a scheduling policy using `setSchedule()`
    - Optionally, dedicate a CPU core to the thread using `setCpuAffinity()`
  - Implement the time-critical step update in `process()`. Additionally, this path should:
    - Avoid locks. Use lock-free atomic variables. Where unavoidable, use priority-inversion safe [`Mutex`](include/grape/realtime/mutex.h)
    - Avoid OS system calls 
    - Avoid third-party code whose worst-case execution time is unknown
    - Avoid additional memory allocation/deallocation. 
    - Avoid I/O (including console output). Use `MPSCQueue` or lock-free ring-buffers as the intermediary to transfer data both ways from the task
    - Avoid algorithms > O(1) in complexity
    - Only use monotonic clock for timing
  - Implement task cleanup before exiting in `teardown()`
  - Consider disabling memory swapping _for the process address space_ with `lockMemory()`, preferably at the top of the application.

See [thread_example.cpp](examples/thread_example.cpp) that illustrates these principles.

Facilities such as a watchdog to monitor the health of the real-time task thread can be implemented in a similar way.  

#### A note on exceptions

- There is no cost to exceptions until they are thrown. Therefore, they may be used in the task thread for indicating unrecoverable errors, where the only sensible option may be to terminate the task. Prefer `grape::Exception` and its derivatives to capture diagnostic information before exit.
- In all other cases, prefer to return `std::expected` from functions over throwing an exception.
 
 An unhandled exception in the task thread terminates the task thread but not the application, giving the main thread some opportunity for damage control.

### OS configuration

- Configure the kernel for preemption by following the [Real-time Ubuntu](https://ubuntu.com/real-time) guide
- (Optional) Configure [RT throttling](https://wiki.linuxfoundation.org/realtime/documentation/technical_basics/sched_rt_throttling) to limit execution time of real-time tasks. The default settings below indicate 95% CPU is set aside for real-time processes:
  ```bash
  # cat /proc/sys/kernel/sched_rt_period_us
  1000000
  # cat /proc/sys/kernel/sched_rt_runtime_us
  950000
  ```
  As an example, to temporarily set 50% CPU usage for real-time tasks and a larger period:
  ```bash
  echo 2000000 > /proc/sys/kernel/sched_rt_period_us
  echo 1000000 > /proc/sys/kernel/sched_rt_runtime_us
  ```
  Setting `sched_rt_runtime_us=-1` disables throttling, and may be desirable in some cases. But, note that this could result in an unresponsive system if a runaway RT task monopolises the CPU. 

  To make the change persistent, modify `/etc/sysctl.conf` as follows and reboot:
  ```
  kernel.sched_rt_runtime_us=-1
  ```
- (Optional) Consider disabling memory swapping. The `swappiness` setting indicates the percentage of free memory before activating swap. The default below indicates swap area may be used by the OS if free memory falls below 60%
  ```bash
  # cat /proc/sys/vm/swappiness
  60
  ```
  Setting `swappiness=0` disables swapping completely:
  ```bash
  echo 0 > /proc/sys/vm/swappiness
  ```
  To make the change persistent, modify `/etc/sysctl.conf` as follows and reboot:
  ```
  vm.swappiness=0
  ```

#### A note on `top` and process priority

`top` displays process priority in the 'PR' column in the range [-100, 39]. Lower PR mean higher process priority. PR is calculated as follows:
- For regular processes: PR = 20 + NI (NI is 'nice' in the range [-20, 19]. Pnemonic: lower NI => process is less 'nice' => takes higher priority over other processes).
- For real time processes: PR = -1 - rt_priority (rt_priority range: [1, 99])

## References

- Canonical's [Real-time Ubuntu](https://ubuntu.com/real-time) page describes how to configure Ubuntu with preemptible kernel. Read their [whitepaper](./media/2023-11-29-ubuntu-rtl-whitepaper.pdf) for background. 
- Some implementation details here are inspired by John Ogness' talk, [A Checklist for Writing Linux Real-Time Applications](docs/media/2020-10-26-john-ogness-rt-checklist.pdf) at Embedded Liux Conference Europe 2020 and a subsequent [commentary](https://lwn.net/Articles/837019/) on it. 
- Timur Doumler's talks on the topic of low-latency are usually good. Here are a few: 
  - [Real-time programming with the C++ standard library](https://youtu.be/Tof5pRedskI)
  - What is low latency C++: [Part 1](https://youtu.be/EzmNeAhWqVs), [Part 2](https://youtu.be/5uIsadq-nyk)
- And see [Realtime Linux Project](https://wiki.linuxfoundation.org/realtime/start) for additional resources
