//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cinttypes>
#include <span>

#include <sys/types.h>

namespace grape::realtime {

/// Lock process address space into physical memory to avoid page faults and resulting unbounded
/// memory access latency. Usually called on application startup and before creation of any threads.
/// @note This function is a no-op on any OS except Linux
void lockMemory();

/// Set CPU affinity for a thread in Linux.
/// @note This function is a no-op on any OS except Linux
/// @param cpus List of CPU indices, starting at 0
/// @param pid Linux specific thread ID, typically returned by gettid() (0=calling thread)
void setCpuAffinity(std::span<const int> cpus, pid_t pid = 0);

/// Thread scheduling parameters
struct Schedule {
  enum class Policy : std::uint8_t { NonRealtime, Realtime };
  Policy policy{ Policy::NonRealtime };
  std::uint8_t priority{ 0 };  //!< Range [0,99]; 0 for NonRealtime, and 1-99 for Realtime
};

/// Set scheduling parameters for a thread
/// @note This function requires root privileges unless the OS is configured to allow certain
/// users/groups. See README.md
/// @param schedule Scheduling parameters.
/// @param pid Linux specific thread ID, typically returned by gettid() (0=calling thread)
void setSchedule(Schedule schedule, pid_t pid = 0);

}  // namespace grape::realtime
