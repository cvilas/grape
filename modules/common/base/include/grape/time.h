//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <format>

namespace grape {

//=================================================================================================
/// Platform-agnostic system clock
///
/// Avoids difference in internal representation across compilers
struct SystemClock {
  using Duration = std::chrono::duration<std::int64_t, std::nano>;
  using TimePoint = std::chrono::time_point<std::chrono::system_clock, Duration>;

  /// @return time now
  [[nodiscard]] static auto now() -> TimePoint {
    return std::chrono::time_point_cast<Duration>(std::chrono::system_clock::now());
  }

  /// Returns a timepoint formatted as nanoseconds since Unix epoch
  [[nodiscard]] static constexpr auto toNanos(const SystemClock::TimePoint& tp) -> std::uint64_t {
    const auto ns = std::chrono::duration_cast<Duration>(tp.time_since_epoch());
    return static_cast<std::uint64_t>(ns.count());
  }

  /// Returns nanoseconds since Unix epoch formatted as a timepoint
  [[nodiscard]] static constexpr auto fromNanos(std::uint64_t nanos) -> SystemClock::TimePoint {
    const auto dur = std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(nanos));
    return SystemClock::TimePoint(dur);
  }
};

}  // namespace grape

//=================================================================================================
// Specializes std::formatter for grape::SystemClock::Duration
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::formatter<grape::SystemClock::Duration> : std::formatter<std::int64_t> {
  auto format(const grape::SystemClock::Duration& dt, std::format_context& ctx) const {
    auto it = std::formatter<std::int64_t>::format(dt.count(), ctx);
    return std::format_to(it, "ns");
  }
};
// NOLINTEND(cert-dcl58-cpp)
