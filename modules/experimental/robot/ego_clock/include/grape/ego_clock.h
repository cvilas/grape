//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <format>

namespace grape {

//=================================================================================================
/// Plant internal process clock
///
/// This clock provides timing with reference to a clock that is custom/internal to the plant or
/// process under control. The clock is driven by periodic ticks from EgoClockDriver, which in turn
/// could be driven by a user-defined source, e.g. a real-time operating system, hardware clock,
/// or physics simulation. Indeed, even an NTP synchronised clock can be used as the source,
/// in which case it just mimics the behaviour of WallClock.
struct EgoClock {
  using Duration = std::chrono::duration<std::int64_t, std::nano>;
  using TimePoint = std::chrono::time_point<EgoClock, Duration>;
  static constexpr bool IS_STEADY = false;

  /// Wait for master (driver) to initialise this clock
  /// @param timeout How long to wait for
  /// @return true if clock was initialsed, false on timeout
  [[nodiscard]] static auto waitForMaster(const std::chrono::milliseconds& timeout) -> bool;

  /// @return Current timestamp
  [[nodiscard]] static auto now() -> EgoClock::TimePoint;

  /// @return nanoseconds since clock epoch, given time point
  [[nodiscard]] static constexpr auto toNanos(const EgoClock::TimePoint& tp) -> std::int64_t {
    const auto ns = std::chrono::duration_cast<Duration>(tp.time_since_epoch());
    return static_cast<std::int64_t>(ns.count());
  }

  /// @return time point given nanoseconds since clock epoch
  [[nodiscard]] static constexpr auto fromNanos(std::int64_t nanos) -> EgoClock::TimePoint {
    const auto dur = std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(nanos));
    return EgoClock::TimePoint(dur);
  }
};

/// sleep until a given time point
void sleepUntil(const EgoClock::TimePoint& tp);

/// sleep for a given duration
void sleepFor(const EgoClock::Duration& dt);

}  // namespace grape

//=================================================================================================
// Specializes std::formatter for grape::EgoClock::TimePoint
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::formatter<grape::EgoClock::TimePoint> {
  static constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  static auto format(const grape::EgoClock::TimePoint& tp, std::format_context& ctx) {
    const auto total_ns = grape::EgoClock::toNanos(tp);
    const auto hours = total_ns / (60LL * 60LL * 1'000'000'000LL);
    const auto remainder_after_hours = total_ns % (60LL * 60LL * 1'000'000'000LL);
    const auto minutes = remainder_after_hours / (60LL * 1'000'000'000LL);
    const auto remainder_after_minutes = remainder_after_hours % (60LL * 1'000'000'000LL);
    const auto seconds = remainder_after_minutes / 1'000'000'000LL;
    const auto nanoseconds = remainder_after_minutes % 1'000'000'000LL;
    return std::format_to(ctx.out(), "{:06}:{:02}:{:02}.{:09}", hours, minutes, seconds,
                          nanoseconds);
  }
};
// NOLINTEND(cert-dcl58-cpp)
