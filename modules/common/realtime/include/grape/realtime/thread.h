//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include <type_traits>

#include "grape/realtime/system_error.h"

namespace grape::realtime {

/// @brief A versatile thread spawner
///
/// Use this class as the fundamental building block for realtime tasks. As demonstrated in
/// the example program, delegate the realtime task to the thread, leaving the main thread for
/// regular tasks such as event handling, I/O, GUI processing, etc.
/// @include thread_example.cpp
class Thread {
public:
  using ProcessClock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
                                          std::chrono::high_resolution_clock,  //
                                          std::chrono::steady_clock>;

  /// @brief Thread configuration parameters
  struct Config {
    /// @brief Function called once immediately on entry into thread. Use this for one-time
    /// configuration of the thread.
    /// @return 'true' to indicate success and continue with the thread; 'false' to indicate
    /// failure, in which case the thread exits immediately without calling teardown()
    std::function<bool()> setup{ [] { return false; } };

    /// @brief Function called periodically (see interval) from within the thread
    /// @return 'true' to indicate success and to continue calling the function in the next
    /// time-step; 'false' to indicate failure or exit condition (will call teardown())
    std::function<bool(const ProcessClock::time_point&)> process{
      [](const ProcessClock::time_point&) { return false; }
    };

    /// @brief Function called once just before exit from thread. Use this for any cleaning up
    /// before exiting thread
    std::function<void()> teardown{ []() {} };

    static constexpr auto DEFAULT_PROCESS_INTERVAL = std::chrono::microseconds(1000'000);

    /// Sets intervals at which 'process' function is triggered
    std::chrono::microseconds interval{ DEFAULT_PROCESS_INTERVAL };

    /// Sets an optional custom name for the thread
    std::string name;
  };

public:
  /// Prepare new thread.
  /// @param config configuration parameters
  explicit Thread(Config&& config);

  /// Start/restart thread.
  void start();

  /// Request to stop thread. Blocks until the thread exits.
  void stop() noexcept;

  ~Thread();
  Thread(const Thread&) = delete;
  Thread(Thread&&) = delete;
  auto operator=(const Thread&) -> Thread& = delete;
  auto operator=(const Thread&&) -> Thread& = delete;

private:
  void threadFunction() noexcept;

  Config config_;
  std::atomic_flag exit_flag_{ false };
  std::thread thread_;
};

//-------------------------------------------------------------------------------------------------
inline Thread::Thread(Thread::Config&& config) : config_(std::move(config)) {
}

//-------------------------------------------------------------------------------------------------
inline Thread::~Thread() {
  stop();
}

//-------------------------------------------------------------------------------------------------
inline void Thread::start() {
  stop();
  exit_flag_.clear();
  thread_ = std::thread([this]() { threadFunction(); });
}

//-------------------------------------------------------------------------------------------------
inline void Thread::stop() noexcept {
  exit_flag_.test_and_set();
  if (thread_.joinable()) {
    thread_.join();
  }
}

//-------------------------------------------------------------------------------------------------
inline void Thread::threadFunction() noexcept {
  try {
#ifdef __linux__
    // for easy identification on tools like htop, set the name of the thread
    if (not config_.name.empty()) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
      std::ignore = ::prctl(PR_SET_NAME, config_.name.c_str());
    }
#endif
    if (not config_.setup()) {
      return;
    }

    bool continue_process = true;
    auto next_wakeup_tp = ProcessClock::now() + config_.interval;
    while (continue_process and not exit_flag_.test()) {
      // wait our turn
      std::this_thread::sleep_until(next_wakeup_tp);

      // run process step and time it
      const auto process_start_tp = ProcessClock::now();
      continue_process = config_.process(process_start_tp);
      const auto process_end_tp = ProcessClock::now();

      // compute how long to sleep this thread until next process step
      const auto wakeup_latency = process_start_tp - next_wakeup_tp;
      const auto process_latency = process_end_tp - process_start_tp;
      if (wakeup_latency + process_latency > config_.interval) {
        // process overran allocated time. reset timing
        next_wakeup_tp = ProcessClock::now();
        // TODO: Log this event, with wakeup and process latencies
      } else {
        next_wakeup_tp += config_.interval;
      }
    }

    config_.teardown();
  } catch (const grape::realtime::SystemException& ex) {
    const auto& context = ex.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "\n(syscall: %s) ", context.function_name.data());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "\nThread '%s' terminated", config_.name.c_str());
    grape::realtime::SystemException::consume();
  } catch (...) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "\nThread '%s' terminated", config_.name.c_str());
    grape::AbstractException::consume();
  }
}
}  // namespace grape::realtime
