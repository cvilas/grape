//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <mutex>
#include <thread>

#include "clock_data_receiver.h"
#include "grape/exception.h"

namespace {
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
std::once_flag s_register_cleanup_flag;
std::optional<grape::ego_clock::ClockDataReceiver> s_receiver{ std::nullopt };
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

//-------------------------------------------------------------------------------------------------
void cleanup() {
  s_receiver.reset();
}

//-------------------------------------------------------------------------------------------------
auto clockReceiver() -> grape::ego_clock::ClockDataReceiver& {
  if (s_receiver) {
    return *s_receiver;
  }
  s_receiver.emplace();

  // Receiver depends on static global state held by the IPC library. Ensure we exi before IPC
  // shared library is unloaded or we may exit with a segfault referencing IPC state during exit.
  std::call_once(s_register_cleanup_flag, []() { (void)std::atexit(cleanup); });
  return *s_receiver;  // NOLINT(bugprone-unchecked-optional-access)
}

//-------------------------------------------------------------------------------------------------
constexpr auto toWallTime(const grape::ego_clock::ClockTransform& tf,
                          const grape::EgoClock::TimePoint& tp) -> grape::WallClock::TimePoint {
  const auto ns = (tf.scale * static_cast<double>(grape::EgoClock::toNanos(tp))) + tf.offset;
  return grape::WallClock::fromNanos(static_cast<std::int64_t>(ns));
}

//-------------------------------------------------------------------------------------------------
constexpr auto toEgoTime(const grape::ego_clock::ClockTransform& tf,
                         const grape::WallClock::TimePoint& tp) -> grape::EgoClock::TimePoint {
  const auto ns = (static_cast<double>(grape::WallClock::toNanos(tp)) - tf.offset) / tf.scale;
  return grape::EgoClock::fromNanos(static_cast<std::int64_t>(ns));
}

//-------------------------------------------------------------------------------------------------
constexpr auto toWallDuration(const grape::ego_clock::ClockTransform& tf,
                              const grape::EgoClock::Duration& dur) -> grape::WallClock::Duration {
  return std::chrono::duration_cast<grape::WallClock::Duration>(dur * tf.scale);
}

}  // namespace

namespace grape {

//-------------------------------------------------------------------------------------------------
auto EgoClock::waitForMaster(const std::chrono::milliseconds& timeout) -> bool {
  const auto until = WallClock::now() + timeout;
  static constexpr auto LOOP_WAIT = std::chrono::milliseconds(1);
  const auto& receiver = clockReceiver();
  while (not receiver.transform() and (WallClock::now() < until)) {
    std::this_thread::sleep_for(LOOP_WAIT);
  }
  return receiver.transform().has_value();
}

//-------------------------------------------------------------------------------------------------
auto EgoClock::now() -> EgoClock::TimePoint {
  const auto tf = clockReceiver().transform();
  if (not tf) {
    panic("No master clock signal");
  }
  return toEgoTime(tf.value(), WallClock::now());
}

//-------------------------------------------------------------------------------------------------
void sleepFor(const EgoClock::Duration& dt) {
  const auto tf = clockReceiver().transform();
  if (not tf) {
    panic("No master clock signal");
  }
  std::this_thread::sleep_for(toWallDuration(tf.value(), dt));
}

//-------------------------------------------------------------------------------------------------
void sleepUntil(const EgoClock::TimePoint& tp) {
  const auto tf = clockReceiver().transform();
  if (not tf) {
    panic("No master clock signal");
  }
  std::this_thread::sleep_until(toWallTime(tf.value(), tp));
}

}  // namespace grape
