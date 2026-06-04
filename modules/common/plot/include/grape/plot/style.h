//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>

namespace grape::plot {

/// A single (x, y) data sample.
struct Sample {
  double x{};
  double y{};
};

/// Plot axis identifiers
enum class AxisId : std::uint8_t { AxisX, AxisY };

/// Symbol styles to mark data points
enum class PointStyle : std::uint8_t {
  None,
  Dot,
  Cross,
  Plus,
  Square,
  Diamond,
  Star,
  Triangle,
  TriangleInverted,
  CrossSquare,
  PlusSquare,
};

/// Line styles to join data points
enum class LineStyle : std::uint8_t {
  None,      //!< Unconnected data points (only symbols)
  Lollipop,  //!< Vertical lines from baseline + optional symbol at tip
  Step,      //!< Staircase: step height is the value of the previous data point
  Line,      //!< Points connected by straight line segments (default)
};

/// RGBA color specification
struct Color {
  std::uint8_t r = 0;
  std::uint8_t g = 0;
  std::uint8_t b = 0;
  std::uint8_t a = 0;
};

}  // namespace grape::plot
