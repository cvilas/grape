//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/realtime.h"

#include <cstring>
#include <format>

#ifdef __linux__
#include <malloc.h>
#endif

#include <sched.h>
#include <sys/mman.h>

#include "grape/exception.h"

namespace grape::realtime {

//-------------------------------------------------------------------------------------------------
void lockMemory() {
  // The implementation here follows from John Ogness' talk, 'A Checklist for Writing Linux
  // Real-Time Applications' at Embedded Liux Conference Europe 2020. See docs.
#ifdef __linux__
  // Lock all current and future process address space into RAM, preventing paging into swap area
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
    panic<Exception>(std::format("mlockall: {}", std::strerror(errno)));
  }

  // Heap trimming: when the amount of contiguous free memory at the top of the  heap  grows
  // sufficiently large, system call (sbrk) is used to release this memory back to the system. The
  // following call disables heap trimming completely. (We avoid system calls at the cost of free
  // RAM not being returned to the OS.)
  if (mallopt(M_TRIM_THRESHOLD, -1) != 1) {
    panic<Exception>("mallopt(M_TRIM_THRESHOLD) failed");
  }

  // Disable the use of mmap for servicing large allocation requests. This leaves glibc with only
  // heap memory for new allocations
  if (mallopt(M_MMAP_MAX, 0) != 1) {
    panic<Exception>("mallopt(M_MMAP_MAX) failed");
  }
#endif
}

//-------------------------------------------------------------------------------------------------
void setCpuAffinity(int cpu, pid_t pid) {
#ifdef __linux__
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(cpu, &mask);
  if (0 != sched_setaffinity(pid, sizeof(mask), &mask)) {
    panic<Exception>(std::format("sched_setaffinity: {}", strerror(errno)));
  }
#else
  (void)cpu;
  (void)pid;
#endif
}

//-------------------------------------------------------------------------------------------------
void setSchedule(Schedule schedule, pid_t pid) {
#ifdef __linux__
  sched_param param{};
  param.sched_priority = schedule.priority;
  const auto prio = (schedule.policy == Schedule::Policy::Realtime ? SCHED_FIFO : SCHED_OTHER);
  if (0 != sched_setscheduler(pid, prio, &param)) {
    panic<Exception>(std::format("sched_setscheduler: {}", strerror(errno)));
  }
#else
  (void)schedule;
  (void)pid;
#endif
}

}  // namespace grape::realtime
