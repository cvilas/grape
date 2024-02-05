//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
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
    const auto err = errno;
    panic<SystemException>(std::strerror(err),
                           SystemError{ .code = err, .function_name = "mlockall: {}" });
  }

  // Heap trimming: when the amount of contiguous free memory at the top of the  heap  grows
  // sufficiently large, system call (sbrk) is used to release this memory back to the system. The
  // following call disables heap trimming completely. (We avoid system calls at the cost of free
  // RAM not being returned to the OS.)
  {
    const auto code = mallopt(M_TRIM_THRESHOLD, -1);
    if (code != 1) {
      panic<SystemException>(
          "Unable to disable memory trimming",
          SystemError{ .code = code, .function_name = "mallopt(M_TRIM_THRESHOLD)" });
    }
  }

  // Disable the use of mmap for servicing large allocation requests. This leaves glibc with only
  // heap memory for new allocations
  {
    const auto code = mallopt(M_MMAP_MAX, 0);
    if (code != 1) {
      panic<SystemException>("Unable to disable mmap for large allocation requrests",
                             SystemError{ .code = code, .function_name = "mallopt(M_MMAP_MAX)" });
    }
  }
#endif
}

//-------------------------------------------------------------------------------------------------
void setCpuAffinity(std::span<const unsigned int> cpus, pid_t pid) {
#ifdef __linux__
  cpu_set_t mask;
  CPU_ZERO(&mask);
  for (const auto i : cpus) {
    CPU_SET(i, &mask);
  }
  if (0 != sched_setaffinity(pid, sizeof(mask), &mask)) {
    const auto err = errno;
    panic<SystemException>(strerror(err),
                           SystemError{ .code = err, .function_name = "sched_setaffinity" });
  }
#else
  (void)cpus;
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
    const auto err = errno;
    panic<SystemException>(strerror(err),
                           SystemError{ .code = err, .function_name = "sched_setscheduler" });
  }
#else
  (void)schedule;
  (void)pid;
#endif
}

}  // namespace grape::realtime
