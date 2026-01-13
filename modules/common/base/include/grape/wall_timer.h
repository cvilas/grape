//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <thread>

#include "grape/wall_clock.h"

namespace grape {

//=================================================================================================
/// Wall clock synchronized periodic timer
///
/// Executes a callback function at a configurable period aligned to wall-clock time. If multiple
/// timers are started with the same period across a distributed system of hosts, their callbacks
/// will fire at exactly the same time regardless of when each timer instance was started.
/// (Note: This requires all hosts to be network time synchronised)
///
class WallTimer {
public:
  using Callback = std::function<void()>;

  /// Constructs a wall-clock-synchronized periodic timer.
  /// @param period The interval between successive callback invocations.
  /// @param cb     The callback to invoke on each tick.
  WallTimer(WallClock::Duration period, Callback cb);

private:
  std::jthread worker_;
};

//-------------------------------------------------------------------------------------------------
inline WallTimer::WallTimer(WallClock::Duration period, Callback cb)
  : worker_([period, callback = std::move(cb)](const std::stop_token& stoken) {
    static constexpr auto EPOCH = WallClock::TimePoint{};
    while (not stoken.stop_requested()) {
      const auto elapsed = WallClock::now() - EPOCH;
      const auto cycles = elapsed / period + 1;
      const auto next_tick = EPOCH + (cycles * period);
      std::this_thread::sleep_until(next_tick);
      if (stoken.stop_requested()) {
        break;
      }
      callback();
    }
  }) {
}

}  // namespace grape
