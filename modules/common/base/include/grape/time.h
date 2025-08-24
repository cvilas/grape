//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>

namespace grape {

//=================================================================================================
/// Platform-agnostic system clock
///
/// Avoids difference in internal representation across compilers
struct SystemClock {
  using Duration = std::chrono::nanoseconds;
  using TimePoint = std::chrono::time_point<std::chrono::system_clock, Duration>;

  [[nodiscard]] static auto now() -> TimePoint {
    return std::chrono::time_point_cast<Duration>(std::chrono::system_clock::now());
  }

  [[nodiscard]] static constexpr auto toNanos(const SystemClock::TimePoint& tp) -> std::uint64_t {
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
    return static_cast<std::uint64_t>(ns.count());
  }

  [[nodiscard]] static constexpr auto fromNanos(std::uint64_t nanos) -> SystemClock::TimePoint {
    const auto dur =
        std::chrono::duration_cast<SystemClock::Duration>(std::chrono::nanoseconds(nanos));
    return SystemClock::TimePoint(dur);
  }
};

}  // namespace grape
