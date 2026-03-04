//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <format>
#include <memory>
#include <optional>
#include <string>

namespace grape {

//=================================================================================================
/// A clock paced by ticks from a ClockDriver instance
///
/// This clock provides timing based on periodic ticks received from a ClockDriver instance.
/// The ClockDriver itself could be driven by a user-defined source, e.g. a real-time operating
/// system, hardware clock, or physics simulation. Indeed, even an NTP synchronised clock can be
/// used as the source, in which case it just mimics the behaviour of WallClock.
///
struct FollowerClock {
  using Duration = std::chrono::duration<std::int64_t, std::nano>;
  using TimePoint = std::chrono::time_point<FollowerClock, Duration>;
  static constexpr bool IS_STEADY = false;

  /// Wait for master (driver) and initialise clock
  /// @param clock_name Unique identifier for clock source
  /// @param timeout How long to wait for
  /// @return An initialised clock, or nothing if timed out waiting for master clock signal
  [[nodiscard]] static auto create(const std::string& clock_name,
                                   const std::chrono::milliseconds& timeout)
      -> std::optional<FollowerClock>;

  /// @return Current timestamp
  [[nodiscard]] auto now() const noexcept -> FollowerClock::TimePoint;

  /// sleep until a given time point
  void sleepUntil(const FollowerClock::TimePoint& tp) const;

  /// sleep for a given duration
  void sleepFor(const FollowerClock::Duration& dt) const;

  /// @return nanoseconds since clock epoch, given time point
  [[nodiscard]] static constexpr auto toNanos(const FollowerClock::TimePoint& tp) -> std::int64_t {
    const auto ns = std::chrono::duration_cast<Duration>(tp.time_since_epoch());
    return static_cast<std::int64_t>(ns.count());
  }

  /// @return time point given nanoseconds since clock epoch
  [[nodiscard]] static constexpr auto fromNanos(std::int64_t nanos) -> FollowerClock::TimePoint {
    const auto dur = std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(nanos));
    return FollowerClock::TimePoint(dur);
  }

  ~FollowerClock();
  FollowerClock(FollowerClock&&) noexcept;
  FollowerClock(const FollowerClock&) = delete;
  auto operator=(const FollowerClock&) = delete;
  auto operator=(FollowerClock&&) = delete;

private:
  struct Impl;
  explicit FollowerClock(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape

//=================================================================================================
// Specializes std::formatter for FollowerClock::TimePoint
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::formatter<grape::FollowerClock::TimePoint> {
  static constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  static auto format(const grape::FollowerClock::TimePoint& tp, std::format_context& ctx) {
    const auto total_ns = grape::FollowerClock::toNanos(tp);
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
