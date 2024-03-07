//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cmath>
#include <csignal>
#include <print>

#include "grape/realtime/schedule.h"
#include "grape/realtime/thread.h"

namespace {

std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

//-------------------------------------------------------------------------------------------------
// signal handler
void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

//-------------------------------------------------------------------------------------------------
// A statistics accumulator
class Profiler {
public:
  struct Stats {
    std::uint64_t num_samples{};
    double abs_max{};
    double mean{};
    double variance{};
  };
  void addSample(double sample);
  [[nodiscard]] auto stats() const -> const Stats&;

private:
  Stats stats_;
  double ssd_{ 0 };  //!< sum of square of differences
};

}  // namespace

//=================================================================================================
// Example program demonstrates the recommended approach to building a realtime application.
// - Delegates realtime execution path to a separate thread. (which just accumulates timing stats)
// - Main thread continues unprivileged and handles events, I/O, user inputs, etc.
auto main() -> int {
  std::ignore = signal(SIGINT, onSignal);
  std::ignore = signal(SIGTERM, onSignal);

  try {
    // disable swap
    const auto is_mem_locked = grape::realtime::lockMemory();
    if (not is_mem_locked) {
      const auto err = is_mem_locked.error();
      std::println("Main thread: {}: {}. Continuing ..", err.function_name, strerror(err.code));
    }

    // create task configurator
    auto task_config = grape::realtime::Thread::Config();

    // set task thread identifier
    task_config.name = "rt_task";

    // set task update interval
    static constexpr auto PROCESS_INTERVAL = std::chrono::microseconds(1000);
    task_config.interval = PROCESS_INTERVAL;

    // Define CPU cores to allocate to non-rt and rt threads. Ideally these should be
    // non-intersecting sets.
    static constexpr auto CPUS_RT = { 0u };
    static constexpr auto CPUS_NON_RT = { 1u, 2u, 3u };

    // Set main thread CPU affinity here. Will assign rt thread CPU affinity in task setup().
    const auto is_cpu_set = grape::realtime::setCpuAffinity(CPUS_NON_RT);
    if (not is_cpu_set) {
      const auto err = is_cpu_set.error();
      std::println("Main thread: {}: {}. Continuing ..", err.function_name, strerror(err.code));
    } else {
      std::println("Main thread: Set to run on CPUs {}", CPUS_NON_RT);
    }

    // set task thread to run on a specific CPU with real-time scheduling policy
    task_config.setup = []() -> bool {
      std::println("Setup started");
      const auto is_cpu_set = grape::realtime::setCpuAffinity(CPUS_RT);
      if (not is_cpu_set) {
        const auto err = is_cpu_set.error();
        std::println("Task thread: {}: {}. Continuing ..", err.function_name, strerror(err.code));
      } else {
        std::println("Task thread: Set to run on CPUs {}", CPUS_RT);
      }

      static constexpr auto RT_PRIORITY = 20;
      const auto is_scheduled =
          grape::realtime::setSchedule({ .policy = grape::realtime::Schedule::Policy::Realtime,  //
                                         .priority = RT_PRIORITY });
      if (not is_scheduled) {
        const auto err = is_scheduled.error();
        std::println("Task thread: {}: {}. Continuing ..", err.function_name, strerror(err.code));
      } else {
        std::println("Task thread: Scheduled to run at RT priority {}", RT_PRIORITY);
      }
      std::println("Setup done");
      return true;
    };

    Profiler profiler;

    // set the periodic process function for the task thread
    task_config.process =
        [&profiler](const grape::realtime::Thread::ProcessClock::time_point& tp) -> bool {
      static auto last_tp = tp;
      const auto dt = std::chrono::duration<double>(tp - last_tp).count();
      last_tp = tp;

      profiler.addSample(dt);
      const auto stats = profiler.stats();
      std::print("\rProcess step={:06d}, dt={:.6f}, max={:.6f}, mean={:.6f}, std.dev.={:.9f}",
                 stats.num_samples, dt, stats.abs_max, stats.mean, std::sqrt(stats.variance));

      return true;
    };

    // set the clean up function for the task thread
    task_config.teardown = []() { std::println("\nTeardown"); };

    // off we go. start the task
    auto task = grape::realtime::Thread(std::move(task_config));
    task.start();

    // main thread continues to handle regular tasks such as event handling
    std::println("\nPress ctrl-c to exit");
    s_exit.wait(false);

    // Send request to exit thread
    task.stop();

    // print results from task before exit
    const auto& stats = profiler.stats();
    std::println(
        "Final process timing statistics: max={:.6f}, mean={:.6f}, std.dev.={:.9f} ({} samples)",
        stats.abs_max, stats.mean, std::sqrt(stats.variance), stats.num_samples);

  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
inline void Profiler::addSample(double sample) {
  // Numerically stable version of Welford's online algorithm from
  // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
  stats_.abs_max = std::max(std::abs(sample), stats_.abs_max);
  stats_.num_samples += 1;
  const auto delta = sample - stats_.mean;
  stats_.mean += delta / static_cast<double>(stats_.num_samples);
  const auto delta2 = sample - stats_.mean;
  ssd_ += delta * delta2;
  stats_.variance = ssd_ / static_cast<double>(stats_.num_samples - 1);
}

//-------------------------------------------------------------------------------------------------
inline auto Profiler::stats() const -> const Profiler::Stats& {
  return stats_;
}
