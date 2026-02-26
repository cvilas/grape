//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <span>

namespace grape::rpi::sense_hat {

//=================================================================================================
/// Interface to the 8x8 RGB LED matrix in the Sense HAT v2
///
class Display {
public:
  struct Coordinate {
    std::uint8_t x;
    std::uint8_t y;
  };

  struct RGB888 {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
  };

  union RGB565 {
    std::uint16_t value;
    struct {
      std::uint16_t r : 5;
      std::uint16_t g : 6;
      std::uint16_t b : 5;
    } bits;
  };

  static constexpr auto WIDTH = 8U;
  static constexpr auto HEIGHT = 8U;
  static constexpr auto NUM_PIXELS = WIDTH * HEIGHT;

  static constexpr auto transform(const RGB888& color) -> RGB565 {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise)
    return RGB565{ .value = static_cast<std::uint16_t>(((color.r >> 3U) << 11U) |
                                                       ((color.g >> 2U) << 5U) | (color.b >> 3U)) };
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,hicpp-signed-bitwise)
  }

  Display();
  void clear();
  void set(const Coordinate& coord, const RGB565& color);
  void set(std::span<const RGB565, NUM_PIXELS> colors);

  ~Display();
  Display(const Display&) = delete;
  Display(Display&&) = delete;
  auto operator=(const Display&) = delete;
  auto operator=(Display&&) = delete;

private:
  [[nodiscard]] auto get(const Coordinate& coord) const -> RGB565;
  [[nodiscard]] auto get() const -> std::span<const RGB565, NUM_PIXELS>;
  [[nodiscard]] auto get() -> std::span<RGB565, NUM_PIXELS>;

  static constexpr auto FB_BYTES = NUM_PIXELS * sizeof(RGB565);
  RGB565* frame_buf_{ nullptr };
};

}  // namespace grape::rpi::sense_hat
