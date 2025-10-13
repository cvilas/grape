//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <cstring>
#include <print>

#include <unistd.h>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/realtime/schedule.h"
#include "grape/utils/format_ranges.h"

//=================================================================================================
/// Utility to run any application with a user specified priority and CPU affinity
/// usage:
/// grape_run_rt --priority=<prio> --cpus=<list,of,cpus> --app=<appln> <appln options>
///
auto main(int argc, char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_RT_PRIORITY = 20;
    static constexpr auto DEFAULT_CPUS = { 1U, 2U };
    const auto args =
        grape::conio::ProgramDescription("Application runner")
            .declareOption<int>("priority", "Application runtime priority", DEFAULT_RT_PRIORITY)
            .declareOption<std::vector<unsigned int>>("cpus", "CPU core to run on", DEFAULT_CPUS)
            .declareOption<std::string>("app", "Application to run, with it's options")
            .parse(argc, const_cast<const char**>(argv));
    const auto cpus = args.getOption<std::vector<unsigned int>>("cpus");
    const auto priority = static_cast<std::uint8_t>(args.getOption<int>("priority"));

    // Set CPU affinity
    const auto cpu_set_err = grape::realtime::setCpuAffinity(cpus);
    if (not cpu_set_err) {
      std::println("Could not set CPU affinity: {}", cpu_set_err.error().message());
      return EXIT_FAILURE;
    }
    std::println("Assigned CPU: {}", cpus);

    // set real-time schedule
    const auto is_scheduled = grape::realtime::setSchedule(
        { .policy = grape::realtime::Schedule::Policy::Realtime, .priority = priority });
    if (not is_scheduled) {
      std::println("Could not set RT schedule: {}", is_scheduled.error().message());
      return EXIT_FAILURE;
    }
    std::println("Assigned RT priority: {}", priority);

    // drop privileges
    if (setuid(getuid()) == -1) {
      std::println("Setuid failed: {}", std::strerror(errno));  // NOLINT(concurrency-mt-unsafe)
      return EXIT_FAILURE;
    }

    // run the application
    const auto app = args.getOption<std::string>("app");
    std::println("Executing application: {}", app);
    auto app_argv = std::array<const char*, 4>{ "/bin/sh", "-c", app.c_str(), nullptr };
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    execvp(app_argv[0], const_cast<char* const*>(app_argv.data()));

    // If execvp returns, it means there was an error
    std::println("Application failed: {}", std::strerror(errno));  // NOLINT(concurrency-mt-unsafe)
    return EXIT_FAILURE;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
