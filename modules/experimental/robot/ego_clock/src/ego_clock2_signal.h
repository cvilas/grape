//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <chrono>
#include <climits>
#include <ctime>
#include <system_error>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace grape::ego_clock {

//=================================================================================================
// Shared memory layout for EgoClock2 futex-based synchronization
struct EgoClock2Signal {
  alignas(std::int64_t) std::atomic<std::int64_t> nanos{ 0 };
  [[nodiscard]] auto get() const -> std::int64_t;
  void post(std::int64_t value);
  [[nodiscard]] auto wait(std::int64_t expected, std::chrono::milliseconds timeout) const -> bool;
};

//-------------------------------------------------------------------------------------------------
inline auto EgoClock2Signal::get() const -> std::int64_t {
  return nanos.load(std::memory_order_acquire);
}

//-------------------------------------------------------------------------------------------------
inline void EgoClock2Signal::post(std::int64_t value) {
  nanos.store(value, std::memory_order_release);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto result = syscall(SYS_futex, &nanos, FUTEX_WAKE, INT_MAX, nullptr, nullptr, 0);
  if (result == -1) {
    const auto err = std::error_code(errno, std::system_category());
    throw std::runtime_error("(futex_wake) " + err.message());
  }
}

//-------------------------------------------------------------------------------------------------
inline auto EgoClock2Signal::wait(std::int64_t expected, std::chrono::milliseconds timeout) const
    -> bool {
  const auto sec = std::chrono::duration_cast<std::chrono::seconds>(timeout);
  const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - sec);
  const auto ts = timespec{ .tv_sec = sec.count(), .tv_nsec = nsec.count() };

  // Futex operates on 32-bit values and we use it only to detect change
  const auto ex = static_cast<std::uint32_t>(expected);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto result = syscall(SYS_futex, &nanos, FUTEX_WAIT, ex, &ts, nullptr, 0);
  if (result == -1) {
    const auto err = std::error_code(errno, std::system_category());
    if (err.value() == ETIMEDOUT) {
      return false;  // timeout
    }
    if (err.value() == EINTR) {
      return false;  // interrupted
    }
    if (err.value() == EAGAIN) {
      return true;  // value already changed
    }
    throw std::runtime_error("(futex_wait) " + err.message());
  }
  return true;
}

}  // namespace grape::ego_clock
