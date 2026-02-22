//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <format>

namespace grape {

//=================================================================================================
/// Global/wall clock.
///
/// Provides timing with reference to an external standard time source (e.g. NTP)
///
/// (This is a platform and compiler agnostic alias for std::chrono::system_clock)
struct WallClock {
  using Duration = std::chrono::duration<std::int64_t, std::nano>;
  using TimePoint = std::chrono::time_point<std::chrono::system_clock, Duration>;
  static constexpr bool IS_STEADY = false;

  /// @return time now
  [[nodiscard]] static auto now() -> TimePoint {
    return std::chrono::time_point_cast<Duration>(std::chrono::system_clock::now());
  }

  /// @return nanoseconds since clock epoch, given time point
  [[nodiscard]] static constexpr auto toNanos(const WallClock::TimePoint& tp) -> std::int64_t {
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
    return static_cast<std::int64_t>(ns.count());
  }

  /// @return time point given nanoseconds since clock epoch
  [[nodiscard]] static constexpr auto fromNanos(std::int64_t nanos) -> WallClock::TimePoint {
    const auto dur = std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(nanos));
    return WallClock::TimePoint(dur);
  }

  /// @return microseconds since clock epoch, given time point
  [[nodiscard]] static constexpr auto toMicros(const WallClock::TimePoint& tp) -> std::int64_t {
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
    return static_cast<std::int64_t>(us.count());
  }

  /// @return time point given microseconds since clock epoch
  [[nodiscard]] static constexpr auto fromMicros(std::int64_t micros) -> WallClock::TimePoint {
    const auto dur = std::chrono::duration_cast<Duration>(std::chrono::microseconds(micros));
    return WallClock::TimePoint(dur);
  }
};

}  // namespace grape

//=================================================================================================
// Specializes std::formatter for grape::WallClock::Duration
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::formatter<grape::WallClock::Duration> : std::formatter<std::int64_t> {
  auto format(const grape::WallClock::Duration& dt, std::format_context& ctx) const {
    auto it = std::formatter<std::int64_t>::format(dt.count(), ctx);
    return std::format_to(it, "ns");
  }
};
// NOLINTEND(cert-dcl58-cpp)
