//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>
#include <string>

#include "grape/plot/trace.h"

namespace grape::plot {

//=================================================================================================
/// 2D realtime plot window.
///
/// User interactions:
/// - Scroll        : zoom X axis
/// - Ctrl + Scroll : zoom Y axis
/// - Double-click  : reset view
/// - Drag legend   : move legend box
/// - F             : toggle FPS counter
class Window {
public:
  static constexpr int DEFAULT_WIDTH = 800;
  static constexpr int DEFAULT_HEIGHT = 600;

  explicit Window(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT,
                  const std::string& title = "Plot");

  void setTitleText(const std::string& title);

  void setAxisText(AxisId id, const std::string& label);

  void enableLegend(bool on);

  [[nodiscard]] auto isLegendEnabled() const -> bool;

  // Traces
  /// Create a named trace and return a reference to it.
  /// @return Reference to the newly created (or existing) trace.
  [[nodiscard]] auto createTrace(const std::string& name) -> Trace&;

  /// Look up an existing trace by name.
  /// @return pointer to trace or nullptr if the name is not found.
  [[nodiscard]] auto trace(const std::string& name) const -> Trace*;

  /// Process events.
  /// @return False when the window has been closed.
  [[nodiscard]] auto processEvents() -> bool;

  /// Render one frame into the window.
  void render();

  /// True while the window has not been closed.
  [[nodiscard]] auto isOpen() const -> bool;

  ~Window();
  Window(const Window&) = delete;
  auto operator=(const Window&) = delete;
  Window(Window&&) = delete;
  auto operator=(Window&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::plot
