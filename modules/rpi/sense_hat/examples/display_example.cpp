//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <array>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/rpi/sense_hat/display.h"

namespace {

using Display = grape::rpi::sense_hat::Display;
using RGB565 = Display::RGB565;
using RGB888 = Display::RGB888;

/// Fill the display with a red/blue checkerboard pattern
void showCheckerboard(Display& display) {
  static constexpr auto RED = Display::transform(RGB888{ .r = 255, .g = 0, .b = 0 });
  static constexpr auto BLUE = Display::transform(RGB888{ .r = 0, .g = 0, .b = 255 });

  for (std::uint8_t row = 0; row < Display::HEIGHT; ++row) {
    for (std::uint8_t col = 0; col < Display::WIDTH; ++col) {
      const auto color = ((col + row) % 2U == 0U) ? RED : BLUE;
      display.set({ .x = col, .y = row }, color);
    }
  }
}

/// Fill the display with a rainbow gradient across the 8 columns
void showRainbow(Display& display) {
  static constexpr auto COLORS = std::array<RGB565, Display::WIDTH>{
    Display::transform(RGB888{ .r = 255, .g = 255, .b = 255 }),  // White
    Display::transform(RGB888{ .r = 127, .g = 0, .b = 255 }),    // Violet
    Display::transform(RGB888{ .r = 75, .g = 0, .b = 130 }),     // Indigo
    Display::transform(RGB888{ .r = 0, .g = 0, .b = 255 }),      // Blue
    Display::transform(RGB888{ .r = 0, .g = 255, .b = 0 }),      // Green
    Display::transform(RGB888{ .r = 255, .g = 255, .b = 0 }),    // Yellow
    Display::transform(RGB888{ .r = 255, .g = 165, .b = 0 }),    // Orange
    Display::transform(RGB888{ .r = 255, .g = 0, .b = 0 })       // Red
  };

  for (std::uint8_t row = 0; row < Display::HEIGHT; ++row) {
    for (std::uint8_t col = 0; col < Display::WIDTH; ++col) {
      display.set({ .x = col, .y = row }, COLORS.at(col));
    }
  }
}

}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    auto display = Display();

    static constexpr auto HOLD = std::chrono::seconds(2);

    std::println("Showing checkerboard...");
    showCheckerboard(display);
    std::this_thread::sleep_for(HOLD);

    std::println("Showing rainbow...");
    showRainbow(display);
    std::this_thread::sleep_for(HOLD);

    std::println("Clearing display.");
    display.clear();

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
