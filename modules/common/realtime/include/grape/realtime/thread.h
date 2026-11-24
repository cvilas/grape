//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#ifdef __linux__
#include <sys/prctl.h>
#endif
#include <type_traits>

#include "grape/exception.h"

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
    std::function<bool()> setup{ [] -> bool { return false; } };

    /// @brief Function called periodically (see interval) from within the thread
    /// @return 'true' to indicate success and to continue calling the function in the next
    /// time-step; 'false' to indicate failure or exit condition (will call teardown())
    std::function<bool()> process{ [] -> bool { return false; } };

    /// @brief Function called once just before exit from thread. Use this for any cleaning up
    /// before exiting thread
    std::function<void()> teardown{ [] -> void {} };

    static constexpr auto DEFAULT_PROCESS_INTERVAL = std::chrono::microseconds(1000'000);

    /// Sets intervals at which 'process' function is triggered
    std::chrono::microseconds interval{ DEFAULT_PROCESS_INTERVAL };

    /// Sets an optional custom name for the thread
    std::string name;
  };

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
  void threadFunction(const std::stop_token& stop_token) noexcept;

  Config config_;
  std::mutex wait_mutex_;
  std::condition_variable_any wait_condition_;
  std::jthread thread_;
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
  thread_ = std::jthread([this](const std::stop_token& stop_token) { threadFunction(stop_token); });
}

//-------------------------------------------------------------------------------------------------
inline void Thread::stop() noexcept {
  thread_.request_stop();
  if (thread_.joinable()) {
    thread_.join();
  }
}

//-------------------------------------------------------------------------------------------------
inline void Thread::threadFunction(const std::stop_token& stop_token) noexcept {
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
    auto wakeup_tp = ProcessClock::now();
    while (continue_process and not stop_token.stop_requested()) {
      continue_process = config_.process();
      const auto now_tp = ProcessClock::now();
      const auto dt = now_tp - wakeup_tp;
      wakeup_tp = (dt > config_.interval) ? now_tp : (wakeup_tp + config_.interval);
      auto lock = std::unique_lock{ wait_mutex_ };
      std::ignore = wait_condition_.wait_until(
          lock, stop_token, wakeup_tp, [&stop_token] { return stop_token.stop_requested(); });
    }

    config_.teardown();
  } catch (...) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "\nThread '%s' terminated", config_.name.c_str());
    grape::Exception::print();
  }
}
}  // namespace grape::realtime
