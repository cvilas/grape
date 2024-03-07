//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/schedule.h"

#include <cerrno>
#ifdef __linux__
#include <malloc.h>
#endif
#include <sched.h>
#include <sys/mman.h>

namespace grape::realtime {

//-------------------------------------------------------------------------------------------------
auto lockMemory() -> std::expected<void, SystemError> {
  // The implementation here follows from John Ogness' talk, 'A Checklist for Writing Linux
  // Real-Time Applications' at Embedded Liux Conference Europe 2020. See docs.
#ifdef __linux__
  // Lock all current and future process address space into RAM, preventing paging into swap area
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
    const auto err = errno;
    return std::unexpected(SystemError{ .code = err, .function_name = "mlockall" });
  }

  // Heap trimming: when the amount of contiguous free memory at the top of the  heap  grows
  // sufficiently large, system call (sbrk) is used to release this memory back to the system. The
  // following call disables heap trimming completely. (We avoid system calls at the cost of free
  // RAM not being returned to the OS.)
  {
    const auto disable = -1;
    const auto code = mallopt(M_TRIM_THRESHOLD, disable);
    if (code != 1) {
      return std::unexpected(
          SystemError{ .code = code, .function_name = "mallopt(M_TRIM_THRESHOLD)" });
    }
  }

  // Disable the use of mmap for servicing large allocation requests. This leaves glibc with only
  // heap memory for new allocations
  {
    const auto disable = 0;
    const auto code = mallopt(M_MMAP_MAX, disable);
    if (code != 1) {
      return std::unexpected(SystemError{ .code = code, .function_name = "mallopt(M_MMAP_MAX)" });
    }
  }
  return {};
#else
  return std::unexpected(SystemError{ .code = ENOSYS, .function_name = "lockMemory" });
#endif
}

//-------------------------------------------------------------------------------------------------
auto setCpuAffinity(std::span<const unsigned int> cpus,
                    pid_t pid) -> std::expected<void, SystemError> {
#ifdef __linux__
  cpu_set_t mask;
  CPU_ZERO(&mask);
  for (const auto i : cpus) {
    CPU_SET(i, &mask);
  }
  if (0 != sched_setaffinity(pid, sizeof(mask), &mask)) {
    const auto err = errno;
    return std::unexpected(SystemError{ .code = err, .function_name = "sched_setaffinity" });
  }
  return {};
#else
  (void)cpus;
  (void)pid;
  return std::unexpected(SystemError{ .code = ENOSYS, .function_name = "setCpuAffinity" });
#endif
}

//-------------------------------------------------------------------------------------------------
auto setSchedule(Schedule schedule, pid_t pid) -> std::expected<void, SystemError> {
#ifdef __linux__
  sched_param param{};
  param.sched_priority = schedule.priority;
  const auto prio = (schedule.policy == Schedule::Policy::Realtime ? SCHED_FIFO : SCHED_OTHER);
  if (0 != sched_setscheduler(pid, prio, &param)) {
    const auto err = errno;
    return std::unexpected(SystemError{ .code = err, .function_name = "sched_setscheduler" });
  }
  return {};
#else
  (void)schedule;
  (void)pid;
  return std::unexpected(SystemError{ .code = ENOSYS, .function_name = "setSchedule" });
#endif
}

}  // namespace grape::realtime
