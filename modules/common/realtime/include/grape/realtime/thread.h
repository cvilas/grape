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
    std::function<bool()> setup{ [] { return false; } };

    /// @brief Function called periodically (see interval) from within the thread
    /// @return 'true' to indicate success and to continue calling the function in the next
    /// time-step; 'false' to indicate failure or exit condition (will call teardown())
    std::function<bool()> process{ [] { return false; } };

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
#ifdef __linux__
  // for easy identification on tools like htop, set the name of the thread
  if (not config_.name.empty()) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = ::prctl(PR_SET_NAME, config_.name.c_str());
  }
#endif
  try {
    if (not config_.setup()) {
      return;
    }

    bool continue_process = true;
    auto next_wakeup_tp = ProcessClock::now();
    while (continue_process and not exit_flag_.test()) {
      continue_process = config_.process();
      next_wakeup_tp += config_.interval;
      std::this_thread::sleep_until(next_wakeup_tp);
    }

    config_.teardown();
  } catch (const grape::SystemException& ex) {
    const auto& context = ex.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "(syscall: %s) ", context.function_name.data());
    grape::SystemException::consume();
  } catch (...) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "Thread '%s' terminated.\n", config_.name.c_str());
    grape::AbstractException::consume();
  }
}
}  // namespace grape::realtime
