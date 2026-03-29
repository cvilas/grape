//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <bit>
#include <chrono>
#include <climits>
#include <ctime>
#include <expected>
#include <string_view>
#include <system_error>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "grape/exception.h"
#include "grape/shared_memory.h"

namespace grape::clock {

//=================================================================================================
// Timing pulse transmitted from broadcaster to followers
class Tick {
public:
  /// @return current tick value
  [[nodiscard]] auto get() const -> std::int64_t;

  /// @brief Signal a tick change
  /// @param value the new tick value
  void post(std::int64_t value);

  /// @brief Wait for a tick change
  /// @param expected expected value of tick
  /// @param timeout How long to wait for
  /// @return true if tick change happened. false if timed out, error otherwise
  [[nodiscard]] auto wait(std::int64_t expected, std::chrono::milliseconds timeout) const
      -> std::expected<bool, std::error_code>;

private:
  alignas(std::int64_t) std::atomic<std::int64_t> nanos_{ 0 };
  static_assert(sizeof(nanos_) == sizeof(int64_t));
};

//=================================================================================================
// SharedMemory region containing a single Tick
struct ShmTick : SharedMemory {
  explicit ShmTick(SharedMemory shm) : SharedMemory(std::move(shm)) {
  }

  static constexpr auto TICK_SIZE = sizeof(Tick);

  static auto shmName(std::string_view id) -> std::string {
    static constexpr std::string_view TICK_SHM_NAME_SUFFIX = "_tick";
    return std::format("/{}{}", id, TICK_SHM_NAME_SUFFIX);
  }

  static auto init(std::string_view id, SharedMemory::Access access) -> SharedMemory {
    // Try to create shared memory. If that fails, try to open. If that fails, give up
    const auto shm_name = shmName(id);
    auto maybe_shm = SharedMemory::create(shm_name, TICK_SIZE, access);
    if (not maybe_shm) {
      const auto prev_msg = std::string{ maybe_shm.error().message() };
      maybe_shm = SharedMemory::open(shm_name, access);
      if (not maybe_shm) {
        const auto current_msg = maybe_shm.error().message();
        panic<Exception>(std::format("Shm create: '{}'; open: '{}'", prev_msg, current_msg));
      }
    }
    return std::move(maybe_shm.value());
  }

  static void cleanup(std::string_view id) {
    std::ignore = SharedMemory::remove(shmName(id));
  }

  auto tick() -> Tick& {
    // mmap guarantees page-aligned memory (≥ 4 KiB), satisfying any alignment ≤ max_align_t.
    static_assert(alignof(Tick) <= alignof(std::max_align_t),
                  "Tick alignment exceeds shm page alignment guarantee");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return *reinterpret_cast<Tick*>(data().data());
  }
};

//-------------------------------------------------------------------------------------------------
inline auto Tick::get() const -> std::int64_t {
  return nanos_.load(std::memory_order_acquire);
}

//-------------------------------------------------------------------------------------------------
inline void Tick::post(std::int64_t value) {
  nanos_.store(value, std::memory_order_release);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto result = syscall(SYS_futex, &nanos_, FUTEX_WAKE, INT_MAX, nullptr, nullptr, 0);
  if (result == -1) {
    const auto err = std::error_code(errno, std::system_category());
    panic<Exception>("(futex_wake) " + err.message());  // EINVAL possible but improbable
  }
}

//-------------------------------------------------------------------------------------------------
inline auto Tick::wait(std::int64_t expected_value, std::chrono::milliseconds timeout) const
    -> std::expected<bool, std::error_code> {
  const auto sec = std::chrono::duration_cast<std::chrono::seconds>(timeout);
  const auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - sec);
  const auto ts = timespec{ .tv_sec = sec.count(), .tv_nsec = nsec.count() };

  // Use the lower 32 bits as futex to signal change. Implicit assumptions:
  // - Consequent ticks passed to post() do not differ exactly by 2^32 (~4.29 sec)
  // - CPU is little-endian (X86_64, Aarch64)
  static_assert(std::endian::native == std::endian::little);

  const auto fut_val = static_cast<std::uint32_t>(expected_value);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto result = syscall(SYS_futex, &nanos_, FUTEX_WAIT, fut_val, &ts, nullptr, 0);
  if (result == -1) {
    const auto err = std::error_code(errno, std::system_category());
    if (err.value() == EAGAIN) {
      return true;  // value already changed
    }
    if (err.value() == ETIMEDOUT) {
      return false;
    }
    return std::unexpected(err);  // only possibility per man pages: EINTR
  }
  return true;
}

}  // namespace grape::clock
