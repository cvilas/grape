//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "grape/splat/examiner_camera.h"
#include "grape/splat/math.h"
#include "grape/splat/splat.h"

namespace grape::splat {

//=================================================================================================
/// Interactive 3D gaussian splat viewer, rendered with the SDL3 GPU API.
///
/// User interactions follow the OpenInventor examiner viewer:
/// - Left drag                     : rotate about focal point (virtual trackball)
/// - Release left button in motion : keep spinning
/// - Middle drag, Ctrl/Shift + Left : pan
/// - Right drag, Left + Middle drag, Ctrl + Middle drag, Ctrl + Shift + Left drag : dolly
/// - Scroll                        : dolly
/// - S, then left click on scene   : seek (set focal point to clicked splat, and approach it)
/// - Esc                           : cancel seek
/// - V                             : view all
/// - H                             : go to home view
/// - Shift + H                     : set home view to current view
class Viewer {
public:
  struct Config {
    static constexpr auto DEFAULT_WIDTH = 1280;
    static constexpr auto DEFAULT_HEIGHT = 800;
    int width{ DEFAULT_WIDTH };    //!< Initial window width (screen coordinates)
    int height{ DEFAULT_HEIGHT };  //!< Initial window height (screen coordinates)
    std::string title{ "Splat Viewer" };
    Vec3 up{ .x = 0.F, .y = -1.F, .z = 0.F };  //!< World direction shown as screen-up in home view
    float fov_y{ ExaminerCamera::DEFAULT_FOV_Y };  //!< Vertical field of view (radians)
  };

  /// Create the viewer window and GPU resources. Throws on failure.
  explicit Viewer(const Config& config);

  /// Replace the scene being viewed, and reset the camera to view all of it
  void setScene(std::vector<Splat> splats);

  /// Process pending user input events
  /// @return False when the window has been closed.
  [[nodiscard]] auto processEvents() -> bool;

  /// Render one frame. Blocks to synchronise with display refresh.
  void render();

  ~Viewer();
  Viewer(const Viewer&) = delete;
  auto operator=(const Viewer&) = delete;
  Viewer(Viewer&&) = delete;
  auto operator=(Viewer&&) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::splat
