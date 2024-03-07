//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/realtime/schedule.h"

//=================================================================================================
auto main() -> int {
  try {
    static constexpr auto CPUS = { 1u, 2u, 3u };

    // Set CPU affinity
    const auto is_cpu_set = grape::realtime::setCpuAffinity(CPUS);
    if (not is_cpu_set) {
      const auto error = is_cpu_set.error();
      std::println("Could not set CPU affinity. {}: {}", error.function_name, strerror(error.code));
      return EXIT_FAILURE;
    }
    std::println("Set to run on CPUs {}", CPUS);

    // set real-time schedule
    static constexpr auto RT_PRIORITY = 20;
    const auto is_scheduled =
        grape::realtime::setSchedule({ .policy = grape::realtime::Schedule::Policy::Realtime,  //
                                       .priority = RT_PRIORITY });
    if (not is_scheduled) {
      const auto error = is_scheduled.error();
      std::println("Could not set RT schedule. {}: {}", error.function_name, strerror(error.code));
      return EXIT_FAILURE;
    }
    std::println("Scheduled to run at RT priority {}", RT_PRIORITY);
    return EXIT_SUCCESS;

  } catch (...) {
    grape::AbstractException::consume();
  }
  return EXIT_SUCCESS;
}