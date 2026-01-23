//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cmath>
#include <format>

#include "grape/ego_clock.h"
#include "grape/wall_clock.h"

namespace grape::ego_clock {

//=================================================================================================
/// Data for fitting correspondance between ego-clock and wall-clock
/// wall_clock_ns = scale * ego_clock_ns + offset
struct ClockTransform {
  double scale{ 1. };
  double offset{ 0. };
  double rmse{ 0. };
};

//-------------------------------------------------------------------------------------------------
constexpr auto toString(const ClockTransform& tf) -> std::string {
  return std::format("scale={}, offset={}, rmse={}", tf.scale, tf.offset, tf.rmse);
}

//-------------------------------------------------------------------------------------------------
constexpr auto toWallTime(const ClockTransform& tf, const EgoClock::TimePoint& tp)
    -> WallClock::TimePoint {
  const auto ns = std::fma(tf.scale, static_cast<double>(EgoClock::toNanos(tp)), tf.offset);
  return grape::WallClock::fromNanos(static_cast<std::int64_t>(ns));
}

//-------------------------------------------------------------------------------------------------
constexpr auto toEgoTime(const ClockTransform& tf, const WallClock::TimePoint& tp)
    -> EgoClock::TimePoint {
  const auto ns = (static_cast<double>(WallClock::toNanos(tp)) - tf.offset) / tf.scale;
  return grape::EgoClock::fromNanos(static_cast<std::int64_t>(ns));
}

//-------------------------------------------------------------------------------------------------
constexpr auto toWallDuration(const ClockTransform& tf, const EgoClock::Duration& dur)
    -> WallClock::Duration {
  return std::chrono::duration_cast<WallClock::Duration>(dur * tf.scale);
}

}  // namespace grape::ego_clock
