# README: realtime

## Brief

Facilities to support deterministic real-time execution

## What is a real-time system?

A real-time system has a deterministic upper-bound on response latency. In autonomous systems, this could be in the order of tens of microseconds to ensure correct and safe operation. To satisfy this constraint, time-critical paths in such applications use:
- no locks (use RT-safe [Mutex](include/grape/realtime/mutex.h) if you need synchronisation without priority-inversion)
- no OS system calls or indeed any third-party code whose worst-case execution time is unknown
- no memory allocation or deallocation (use `lockMemory()`)
- no I/O (use ring-buffers or producer-consumer queues, eg: `MPSCQueue`, and interact with I/O on the consumer-side)
- no algorithms > O(1) in complexity
- only monotonic clock for timing

Timeliness is further improved for real-time threads by 
- dedicating a CPU core to each such thread (use `setCpuAffinity()`)
- assigning them real-time scheduling policy (use `setSchedule()`) 
- configuring the OS kernel for preemption. See a later section on OS configuration.

### A note on exceptions

There is no cost to exceptions in a real-time path until they are thrown. Therefore, use them for unrecoverable errors where the only sensible option is to terminate the application. Prefer `grape::Exception` and its derivatives to capture diagnostic information.

In all other situations, prefer to return `std::expected` over throwing an exception. 

## OS configuration

- Configure the kernel for preemption by following these guidelines for [Real-time Ubuntu](https://ubuntu.com/real-time)
- By default, a process can change it's own scheduling policy and priority (eg: by calling `setSchedule()`) only if run as root. This behaviour can be changed in two ways:
  - Grant the ability to a specific executable using `setcap`:
    ```bash
    sudo setcap cap_sys_nice=ep /path/to/executable
    ```
  - Grant the ability to any process run by a specific user/group by modifying `/etc/security/limits.conf`. As an example, the following lines define a _soft_ limit of 20 and a _hard_ limit of 90 for processes run by members of the group `<groupname>` and a _hard_ limit of 50 for processes run by user `<username>` (replace `<username>` and `<groupname>` as needed). 
    ```bash
    @<groupname> hard rtprio 90
    @<groupname> soft rtprio 20
    <username> hard rtprio 50
    ```
    The _soft_ limit is advisory, and _hard_ limit is enforced by the system. For details, `man 5 limits.conf`.
- Optionally, configure [RT throttling](https://wiki.linuxfoundation.org/realtime/documentation/technical_basics/sched_rt_throttling) to limit execution time of realtime tasks. The default settings below indicate 95% CPU is set aside for realtime processes:
  ```bash
  # cat /proc/sys/kernel/sched_rt_period_us
  1000000
  # cat /proc/sys/kernel/sched_rt_runtime_us
  950000
  ```
  To set 50% CPU usage for real-time tasks and a larger period, for instance, one can do:
  ```bash
  echo 2000000 > /proc/sys/kernel/sched_rt_period_us
  echo 1000000 > /proc/sys/kernel/sched_rt_runtime_us
  ```
  Setting `sched_rt_runtime_us=-1` disables throttling. Note that this could cause a runaway RT task to monopolise the CPU and lock up the system. 

  To make the change persistent, modify `/etc/sysctl.conf` as follows:
  ```
  kernel.sched_rt_runtime_us=-1
  ```
  
### A note on `top` and process priority

`top` displays process priority in the 'PR' column in the range [-100, 39]. Lower PR mean higher process priority. PR is calculated as follows:
- For regular processes: PR = 20 + NI (NI is 'nice' in the range [-20, 19]. Pnemonic: lower number = process is less 'nice' = takes higher priority over other processes).
- For real time processes: PR = -1 - rt_priority (rt_priority range: [1, 99])

## References

- Canonical's [Real-time Ubuntu](https://ubuntu.com/real-time) page describes how to configure Ubuntu with preemptible kernel. Read their [whitepaper](./media/2023-11-29-ubuntu-rtl-whitepaper.pdf) for background. 
- Some implementation details here are inspired by John Ogness' talk, [A Checklist for Writing Linux Real-Time Applications](docs/media/2020-10-26-john-ogness-rt-checklist.pdf) at Embedded Liux Conference Europe 2020 and a subsequent [commentary](https://lwn.net/Articles/837019/) on it. 
- Timur Doumler's talks on the topic of low-latency are usually good. Here are a few: 
  - [Real-time programming with the C++ standard library](https://youtu.be/Tof5pRedskI)
  - What is low latency C++: [Part 1](https://youtu.be/EzmNeAhWqVs), [Part 2](https://youtu.be/5uIsadq-nyk)
- And see [Realtime Linux Project](https://wiki.linuxfoundation.org/realtime/start) for additional resources
