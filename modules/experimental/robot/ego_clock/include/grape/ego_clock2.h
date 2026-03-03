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
/// Plant internal process clock (futex-based implementation)
///
/// Provides timing with reference to a clock that is custom/internal to the plant or process under
/// control. Unlike EgoClock, this implementation uses futex-based synchronization over shared
/// memory for direct, low-latency clock tick delivery from EgoClock2Driver, without line fitting or
/// IPC broadcasting.
struct EgoClock2 {
  using Duration = std::chrono::duration<std::int64_t, std::nano>;
  using TimePoint = std::chrono::time_point<EgoClock2, Duration>;
  static constexpr bool IS_STEADY = false;

  /// Wait for driver and initialise clock
  /// @param clock_name Shared memory name for the clock source (must start with '/')
  /// @param timeout How long to wait for the first clock tick
  /// @return An initialised clock, or nothing if timed out waiting for driver
  [[nodiscard]] static auto create(const std::string& clock_name,
                                   const std::chrono::milliseconds& timeout)
      -> std::optional<EgoClock2>;

  /// @return Most recently posted clock value
  [[nodiscard]] auto now() const noexcept -> EgoClock2::TimePoint;

  /// Sleep until a given ego clock time point
  void sleepUntil(const EgoClock2::TimePoint& tp) const;

  /// Sleep for a given ego clock duration
  void sleepFor(const EgoClock2::Duration& dt) const;

  /// @return nanoseconds since clock epoch, given time point
  [[nodiscard]] static constexpr auto toNanos(const EgoClock2::TimePoint& tp) -> std::int64_t {
    const auto ns = std::chrono::duration_cast<Duration>(tp.time_since_epoch());
    return static_cast<std::int64_t>(ns.count());
  }

  /// @return time point given nanoseconds since clock epoch
  [[nodiscard]] static constexpr auto fromNanos(std::int64_t nanos) -> EgoClock2::TimePoint {
    const auto dur = std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(nanos));
    return EgoClock2::TimePoint(dur);
  }

  ~EgoClock2();
  EgoClock2(EgoClock2&&) noexcept;
  EgoClock2(const EgoClock2&) = delete;
  auto operator=(const EgoClock2&) = delete;
  auto operator=(EgoClock2&&) = delete;

private:
  struct Impl;
  explicit EgoClock2(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_{ nullptr };
};

}  // namespace grape

//=================================================================================================
// Specializes std::formatter for grape::EgoClock2::TimePoint
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct std::formatter<grape::EgoClock2::TimePoint> {
  static constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  static auto format(const grape::EgoClock2::TimePoint& tp, std::format_context& ctx) {
    const auto total_ns = grape::EgoClock2::toNanos(tp);
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
