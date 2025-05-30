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
    const auto cpu_set_err = grape::realtime::setCpuAffinity(CPUS);
    if (cpu_set_err.code) {
      std::println("Could not set CPU affinity. {}: {}", cpu_set_err.function_name,
                   cpu_set_err.code.message());
      return EXIT_FAILURE;
    }
    std::println("Set to run on CPUs {}", CPUS);

    // set real-time schedule
    static constexpr auto RT_PRIORITY = 20;
    const auto is_scheduled = grape::realtime::setSchedule(
        { .policy = grape::realtime::Schedule::Policy::Realtime, .priority = RT_PRIORITY });
    if (is_scheduled.code) {
      std::println("Could not set RT schedule. {}: {}", is_scheduled.function_name,
                   is_scheduled.code.message());
      return EXIT_FAILURE;
    }
    std::println("Scheduled to run at RT priority {}", RT_PRIORITY);
    return EXIT_SUCCESS;

  } catch (...) {
    grape::Exception::print();
  }
  return EXIT_SUCCESS;
}
