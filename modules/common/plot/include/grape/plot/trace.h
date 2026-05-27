//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "grape/fifo_buffer.h"
#include "grape/plot/snapshot_buffer.h"
#include "grape/plot/style.h"

namespace grape::plot {

class Window;

//=================================================================================================
/// A single 2D data trace rendered inside a Window.
class Trace {
public:
  // Name
  [[nodiscard]] auto name() const -> const std::string&;

  // line style
  void setLineStyle(LineStyle style);
  [[nodiscard]] auto lineStyle() const -> LineStyle;

  // point style
  void setPointStyle(PointStyle style);
  [[nodiscard]] auto pointStyle() const -> PointStyle;

  // color
  void setColor(Color col);
  [[nodiscard]] auto color() const -> Color;

  /// Append a single (x, y) data point. Writer thread only.
  void addData(const Sample& sample);

private:
  friend class Window;

  /// @brief Construct a new Trace object.
  /// @param name The name of the trace.
  /// @param max_history The maximum samples to retain in a snapshot.
  explicit Trace(std::string name, std::size_t max_history);

  /// View of a trace snapshot (entire history). Samples are split into two contiguous spans in
  /// chronological order; the second span is empty when the buffer hasn't wrapped.
  struct View {
    std::string_view name;
    std::pair<std::span<const Sample>, std::span<const Sample>> samples;
    Color color;
    LineStyle line_style;
    PointStyle point_style;
  };

  /// Get a snapshot view of this trace. Only call from renderer thread. The returned view is valid
  /// until the next call to snapshot().
  auto snapshot() -> View;

  static constexpr auto FIFO_CAPACITY = 1024;
  FIFOBuffer front_buffer_;
  std::string name_;
  SnapshotBuffer back_buffer_;
  LineStyle line_style_{ LineStyle::Line };
  PointStyle point_style_{ PointStyle::None };
  Color color_{};
};

}  // namespace grape::plot
