//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/wall_timer.h"

//=================================================================================================
auto main() -> int {
  static constexpr auto PERIOD = std::chrono::milliseconds(250);

  auto timer = grape::WallTimer(PERIOD, []() {
    const auto now = grape::WallClock::now();
    std::println("Blink! {}", now);
  });

  std::this_thread::sleep_for(std::chrono::seconds(2));
  return EXIT_SUCCESS;
}
