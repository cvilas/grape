//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cmath>
#include <csignal>
#include <print>

#include "grape/realtime/realtime.h"
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
// - Delegates the realtime execution path to the separate thread. (which just accumulates timing
// statistics)
// - Main thread continues unprivileged and handles events, I/O, user inputs, etc.
auto main() -> int {
  std::ignore = signal(SIGINT, onSignal);
  std::ignore = signal(SIGTERM, onSignal);

  try {
    // disable swap
    grape::realtime::lockMemory();

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
    grape::realtime::setCpuAffinity(CPUS_NON_RT);

    // set task thread to run on a specific CPU with real-time scheduling policy
    task_config.setup = []() -> bool {
      std::println("setup() called");
      grape::realtime::setCpuAffinity(CPUS_RT);
      static constexpr auto RT_PRIORITY = 20;
      grape::realtime::setSchedule({ .policy = grape::realtime::Schedule::Policy::Realtime,  //
                                     .priority = RT_PRIORITY });
      return true;
    };

    Profiler profiler;

    // set the periodic process function for the task thread
    task_config.process = [&profiler]() -> bool {
      const auto tp = grape::realtime::Thread::ProcessClock::now();
      static auto last_tp = tp;
      const auto dt = std::chrono::duration<double>(tp - last_tp);
      last_tp = tp;

      profiler.addSample(dt.count());
      const auto stats = profiler.stats();
      std::println("Process timing statistics: max={}, mean={}, std.dev.={} ({} samples) ",
                   std::chrono::duration<double>(stats.abs_max),
                   std::chrono::duration<double>(stats.mean),
                   std::chrono::duration<double>(std::sqrt(stats.variance)), stats.num_samples);

      return true;
    };

    // set the clean up function for the task thread
    task_config.teardown = []() { std::println("teardown() called"); };

    // off we go. start the task
    auto task = grape::realtime::Thread(std::move(task_config));
    task.start();

    // main thread continues to handle regular tasks such as event handling
    std::println("Press ctrl-c to exit");
    s_exit.wait(false);

    // Send request to exit thread
    task.stop();

    // print results from task before exit
    const auto& stats = profiler.stats();
    std::println("Final process timing statistics: max={}, mean={}, std.dev.={} ({} samples) ",
                 std::chrono::duration<double>(stats.abs_max),
                 std::chrono::duration<double>(stats.mean),
                 std::chrono::duration<double>(std::sqrt(stats.variance)), stats.num_samples);

  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
  }
  return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
inline void Profiler::addSample(double sample) {
  // Numerically stable version of Welford's online algorithm from
  // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
  const auto abs = std::abs(sample);
  if (stats_.abs_max < abs) {
    stats_.abs_max = abs;
  }
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
