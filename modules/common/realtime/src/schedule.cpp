//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/realtime/schedule.h"

#include <cerrno>
#ifdef __linux__
#include <malloc.h>
#endif
#include <system_error>

#include <sched.h>
#include <sys/mman.h>

namespace grape::realtime {

//-------------------------------------------------------------------------------------------------
auto lockMemory() -> std::expected<void, Error> {
  // The implementation here follows from John Ogness' talk, 'A Checklist for Writing Linux
  // Real-Time Applications' at Embedded Liux Conference Europe 2020. See docs.
#ifdef __linux__
  // Lock all current and future process address space into RAM, preventing paging into swap area
  if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(mclockall) ", err.message() });
  }

  // Heap trimming: when the amount of contiguous free memory at the top of the  heap  grows
  // sufficiently large, system call (sbrk) is used to release this memory back to the system. The
  // following call disables heap trimming completely. (We avoid system calls at the cost of free
  // RAM not being returned to the OS.)
  {
    const auto disable = -1;
    const auto code = mallopt(M_TRIM_THRESHOLD, disable);  // NOLINT(concurrency-mt-unsafe)
    if (code != 1) {
      const auto err = std::error_code(code, std::system_category());
      return std::unexpected(Error{ "(mallopt(M_TRIM_THRESHOLD)) ", err.message() });
    }
  }

  // Disable the use of mmap for servicing large allocation requests. This leaves glibc with only
  // heap memory for new allocations
  {
    const auto disable = 0;
    const auto code = mallopt(M_MMAP_MAX, disable);  // NOLINT(concurrency-mt-unsafe)
    if (code != 1) {
      const auto err = std::error_code(code, std::system_category());
      return std::unexpected(Error{ "(mallopt(M_MMAP_MAX)) ", err.message() });
    }
  }
  return {};
#else
  const auto err = std::make_error_code(std::errc::function_not_supported);
  return std::unexpected(Error{ "(lockMemory) ", err.message() });
#endif
}

//-------------------------------------------------------------------------------------------------
auto setCpuAffinity(std::span<const unsigned int> cpus, pid_t pid) -> std::expected<void, Error> {
#ifdef __linux__
  cpu_set_t mask;
  CPU_ZERO(&mask);
  for (const auto cpu_no : cpus) {
    CPU_SET(cpu_no, &mask);
  }
  if (0 != sched_setaffinity(pid, sizeof(mask), &mask)) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(sched_setaffinity) ", err.message() });
  }
  return {};
#else
  (void)cpus;
  (void)pid;
  const auto err = std::make_error_code(std::errc::function_not_supported);
  return std::unexpected(Error{ "(setCpuAffinity) ", err.message() });
#endif
}

//-------------------------------------------------------------------------------------------------
auto setSchedule(Schedule schedule, pid_t pid) -> std::expected<void, Error> {
#ifdef __linux__
  sched_param param{};
  param.sched_priority = schedule.priority;
  const auto prio = (schedule.policy == Schedule::Policy::Realtime ? SCHED_FIFO : SCHED_OTHER);
  if (0 != sched_setscheduler(pid, prio, &param)) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected(Error{ "(sched_setscheduler) ", err.message() });
  }
  return {};
#else
  (void)schedule;
  (void)pid;
  const auto err = std::make_error_code(std::errc::function_not_supported);
  return std::unexpected(Error{ "(setScrhedule) ", err.message() });
#endif
}

}  // namespace grape::realtime
