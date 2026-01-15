//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>

namespace grape::rpi::sense_hat {

class MatrixDisplay {
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

  static constexpr auto transform(const RGB888& color) -> RGB565;
  static constexpr auto transform(const RGB565& color) -> RGB888;

  /// enumerate and create the first available display
  /// code: https://github.com/copilot/share/c27f120e-41e4-8c64-8913-3a40e44d2950
  auto create() -> std::optional<MatrixDisplay>;
  void clear();
  void set(const Coordinate& coord, const RGB565& color);
  void set(std::span<const RGB565>);
  [[nodiscard]] auto get(const Coordinate& coord) const -> RGB565;
  [[nodiscard]] auto get() const -> std::span<const RGB565>;
  [[nodiscard]] auto get() -> std::span<RGB565>;

  void dim(bool);

private:
  MatrixDisplay() = default;
};

}  // namespace grape::rpi::sense_hat
