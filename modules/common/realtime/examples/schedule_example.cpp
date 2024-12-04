//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstring>
#include <print>

#include "grape/exception.h"
#include "grape/realtime/schedule.h"

//=================================================================================================
auto main() -> int {
  try {
    static constexpr auto CPUS = { 1U, 2U, 3U };

    // Set CPU affinity
    const auto is_cpu_set = grape::realtime::setCpuAffinity(CPUS);
    if (is_cpu_set.code != 0) {
      std::println("Could not set CPU affinity. {}: {}",  //
                   is_cpu_set.function_name,              //
                   strerror(is_cpu_set.code));            // NOLINT(concurrency-mt-unsafe)
      return EXIT_FAILURE;
    }
    std::println("Set to run on CPUs {}", CPUS);

    // set real-time schedule
    static constexpr auto RT_PRIORITY = 20;
    const auto is_scheduled = grape::realtime::setSchedule(
        { .policy = grape::realtime::Schedule::Policy::Realtime, .priority = RT_PRIORITY });
    if (is_scheduled.code != 0) {
      std::println("Could not set RT schedule. {}: {}",  //
                   is_scheduled.function_name,           //
                   strerror(is_scheduled.code));         // NOLINT(concurrency-mt-unsafe)
      return EXIT_FAILURE;
    }
    std::println("Scheduled to run at RT priority {}", RT_PRIORITY);
    return EXIT_SUCCESS;

  } catch (...) {
    grape::Exception::print();
  }
  return EXIT_SUCCESS;
}
