//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numbers>
#include <vector>

#include "grape/exception.h"
#include "grape/splat/math.h"
#include "grape/splat/splat.h"
#include "grape/splat/viewer.h"

namespace {

//-------------------------------------------------------------------------------------------------
/// Generate a procedural scene: a sphere tiled with flat, oriented gaussian 'scales' coloured by
/// latitude, sitting on a ring of round gaussians
auto makeScene() -> std::vector<grape::splat::Splat> {
  using grape::splat::Quat;
  using grape::splat::Rgba;
  using grape::splat::Splat;
  using grape::splat::Vec3;
  using std::numbers::pi_v;

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  static constexpr auto NUM_LAT = 40;
  static constexpr auto NUM_LON = 80;
  static constexpr auto RADIUS = 1.F;
  auto splats = std::vector<Splat>{};
  for (auto i = 0; i < NUM_LAT; ++i) {
    const auto lat =
        pi_v<float> * (static_cast<float>(i) + 0.5F) / static_cast<float>(NUM_LAT);  // [0, π]
    for (auto j = 0; j < NUM_LON; ++j) {
      const auto lon = 2.F * pi_v<float> * static_cast<float>(j) / static_cast<float>(NUM_LON);
      const auto normal = Vec3{
        .x = std::sin(lat) * std::cos(lon),
        .y = std::cos(lat),
        .z = std::sin(lat) * std::sin(lon),
      };
      // Rotate local Z onto the surface normal, so the flat axis faces outwards
      const auto axis = cross(Vec3{ .z = 1.F }, normal);
      const auto angle = std::acos(normal.z);
      const auto hue = static_cast<float>(i) / static_cast<float>(NUM_LAT);
      const auto tangent_scale = 0.6F * RADIUS * pi_v<float> / static_cast<float>(NUM_LAT);
      splats.push_back({
          .position = RADIUS * normal,
          .scale = { .x = tangent_scale, .y = tangent_scale, .z = 0.005F },
          .rotation = fromAxisAngle(axis, angle),
          .color = {
              .r = static_cast<std::uint8_t>(255.F * hue),
              .g = static_cast<std::uint8_t>(255.F * (1.F - hue)),
              .b = 200,
              .a = 230,
          },
      });
    }
  }
  static constexpr auto NUM_RING = 120;
  for (auto k = 0; k < NUM_RING; ++k) {
    const auto theta = 2.F * pi_v<float> * static_cast<float>(k) / static_cast<float>(NUM_RING);
    splats.push_back({
        .position = { .x = 1.6F * std::cos(theta), .y = -1.F, .z = 1.6F * std::sin(theta) },
        .scale = { .x = 0.05F, .y = 0.05F, .z = 0.05F },
        .rotation = Quat{ .w = 1.F, .x = 0.F, .y = 0.F, .z = 0.F },
        .color = Rgba{ .r = 255, .g = 220, .b = 120, .a = 255 },
    });
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
  return splats;
}

}  // namespace

//=================================================================================================
/// Demonstrates the splat viewer with a procedurally generated scene
auto main() -> int {
  try {
    auto viewer = grape::splat::Viewer(
        { .title = "Splat viewer example", .up = { .x = 0.F, .y = 1.F, .z = 0.F } });
    viewer.setScene(makeScene());
    while (viewer.processEvents()) {
      viewer.render();
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
