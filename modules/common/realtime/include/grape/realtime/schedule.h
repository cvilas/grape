//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <span>

#include <sys/types.h>

#include "grape/realtime/system_error.h"

namespace grape::realtime {

/// Lock process address space into physical memory to avoid page faults and resulting unbounded
/// memory access latency. Usually called on application startup and before creation of any threads.
/// @note This function is a no-op on any OS except Linux
/// @return 0 error code, else information on failure
[[nodiscard]] auto lockMemory() -> SystemError;

/// Set CPU affinity for a thread in Linux.
/// @note This function is a no-op on any OS except Linux
/// @param cpus List of CPU indices, starting at 0
/// @param pid Linux specific thread ID, typically returned by gettid() (0=calling thread)
/// @return 0 error code, else information on failure
[[nodiscard]] auto setCpuAffinity(std::span<const unsigned int> cpus, pid_t pid = 0) -> SystemError;

/// Thread scheduling parameters
struct Schedule {
  enum class Policy : std::uint8_t { NonRealtime, Realtime };
  Policy policy{ Policy::NonRealtime };
  std::uint8_t priority{ 0 };  //!< Range [0,99]; 0 for NonRealtime, and 1-99 for Realtime
};

/// Set scheduling parameters for a thread
/// @note By default, a process can change it's own scheduling policy and priority only if run as
/// root. This behaviour can be changed in two ways:
///  - Grant the ability to a specific executable using `setcap`:
///    ```bash
///    sudo setcap cap_sys_nice=ep /path/to/executable
///    ```
///  - Grant the ability to any process run by a specific user/group by modifying
///  `/etc/security/limits.conf`. For instance, the following lines define a _soft_ limit of 20 and
///  a _hard_ limit of 90 for processes run by members of the group `<groupname>` and a _hard_ limit
///  of 50 for processes run by user `<username>` (replace `<username>` and `<groupname>` as
///  needed).
///    ```bash
///    @<groupname> hard rtprio 90
///    @<groupname> soft rtprio 20
///    <username> hard rtprio 50
///    ```
///    The _soft_ limit is advisory, and _hard_ limit is enforced by the system. For details, `man 5
///    limits.conf`.
/// @param schedule Scheduling parameters.
/// @param pid Linux specific thread ID, typically returned by gettid() (0=calling thread)
/// @return 0 error code, else information on failure
[[nodiscard]] auto setSchedule(Schedule schedule, pid_t pid = 0) -> SystemError;

}  // namespace grape::realtime
