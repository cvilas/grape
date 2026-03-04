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

#include "grape/log/syslog.h"
#include "grape/realtime/shared_memory.h"

namespace grape::clock {

//=================================================================================================
// Timing pulse transmitted from clock driver to follower clocks
struct Tick {
  alignas(std::int64_t) std::atomic<std::int64_t> nanos{ 0 };
  [[nodiscard]] auto get() const -> std::int64_t;
  void post(std::int64_t value);
  [[nodiscard]] auto wait(std::int64_t expected, std::chrono::milliseconds timeout) const -> bool;
};

constexpr auto SHM_NAME_SUFFIX = "/tick";
constexpr auto SHM_SIZE = sizeof(Tick);

//=================================================================================================
// Shared memory where the tick resides
inline auto initShm(const std::string& id, realtime::SharedMemory::Access access)
    -> std::optional<realtime::SharedMemory> {
  // Try to create shared memory. If that fails, try to open. If that fails, give up
  const auto shm_name = id + SHM_NAME_SUFFIX;
  auto maybe_shm = realtime::SharedMemory::create(shm_name, SHM_SIZE, access);
  const auto shm_created = maybe_shm.has_value();
  if (not shm_created) {
    syslog::Note("Creating shared memory for ticks did not succeed: {}",
                 maybe_shm.error().message());
    maybe_shm = realtime::SharedMemory::open(shm_name, access);
    if (not maybe_shm) {
      syslog::Note("Opening shared memory for ticks did not succeed: {}",
                   maybe_shm.error().message());
      return std::nullopt;
    }
  }
  syslog::Info("Got shared memory for ticks");
  return std::move(maybe_shm.value());
}

//-------------------------------------------------------------------------------------------------
inline auto Tick::get() const -> std::int64_t {
  return nanos.load(std::memory_order_acquire);
}

//-------------------------------------------------------------------------------------------------
inline void Tick::post(std::int64_t value) {
  nanos.store(value, std::memory_order_release);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto result = syscall(SYS_futex, &nanos, FUTEX_WAKE, INT_MAX, nullptr, nullptr, 0);
  if (result == -1) {
    const auto err = std::error_code(errno, std::system_category());
    throw std::runtime_error("(futex_wake) " + err.message());
  }
}

//-------------------------------------------------------------------------------------------------
inline auto Tick::wait(std::int64_t expected, std::chrono::milliseconds timeout) const -> bool {
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

}  // namespace grape::clock
