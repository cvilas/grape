//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <format>
#include <memory>
#include <string>

namespace grape::clock {

//=================================================================================================
/// A clock paced by ticks from a ClockBroadcaster instance on the same host
///
/// This clock provides timing based on periodic ticks received from a ClockBroadcaster on the host.
/// The ClockBroadcaster itself must be driven by a user-defined source, e.g. an OS clock, hardware
/// clock, or physics simulation.
///
struct FollowerClock {
  using Duration = std::chrono::duration<std::int64_t, std::nano>;
  using TimePoint = std::chrono::time_point<FollowerClock, Duration>;
  static constexpr bool IS_STEADY = false;

  /// Initialise clock
  /// @param source_name Unique identifier of a clock broadcaster to listen to
  explicit FollowerClock(const std::string& source_name);

  /// Wait for next tick from the broadcaster, or until timeout.
  /// Use-cases:
  /// - Wait for broadcaster to come alive after starting an application
  /// - Detect broadcaster liveliness mid-operation (e.g. within a watchdog)
  /// @param timeout Maximum time to wait for a tick before returning.
  /// @return true if a tick was received, false if timeout occurred.
  [[nodiscard]] auto waitForNextTick(std::chrono::milliseconds timeout) const -> bool;

  /// @return Current timestamp
  [[nodiscard]] auto now() const noexcept -> FollowerClock::TimePoint;

  /// sleep until a given time point or interrupt
  void sleepUntil(const FollowerClock::TimePoint& tp) const;

  /// sleep for a given duration
  void sleepFor(const FollowerClock::Duration& dt) const;

  /// @return nanoseconds since clock epoch, given time point
  [[nodiscard]] static constexpr auto toNanos(const FollowerClock::TimePoint& tp) -> std::int64_t {
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
    return static_cast<std::int64_t>(ns.count());
  }

  /// @return time point given nanoseconds since clock epoch
  [[nodiscard]] static constexpr auto fromNanos(std::int64_t nanos) -> FollowerClock::TimePoint {
    const auto dur = std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(nanos));
    return FollowerClock::TimePoint(dur);
  }

  ~FollowerClock();
  FollowerClock(FollowerClock&&) noexcept = default;
  FollowerClock(const FollowerClock&) = delete;
  auto operator=(const FollowerClock&) = delete;
  auto operator=(FollowerClock&&) noexcept -> FollowerClock& = default;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape::clock

//=================================================================================================
// Specializes std::formatter for FollowerClock::TimePoint
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::formatter<grape::clock::FollowerClock::TimePoint> {
  static constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  static auto format(const grape::clock::FollowerClock::TimePoint& tp, std::format_context& ctx) {
    const auto total_ns = grape::clock::FollowerClock::toNanos(tp);
    const auto dur = std::chrono::nanoseconds(total_ns);
    const auto hms = std::chrono::hh_mm_ss(dur);
    return std::format_to(ctx.out(), "{:06}:{:02}:{:02}.{:09}", hms.hours().count(),
                          hms.minutes().count(), hms.seconds().count(), hms.subseconds().count());
  }
};
// NOLINTEND(cert-dcl58-cpp)
