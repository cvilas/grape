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
        std::sin(lat) * std::cos(lon),
        std::cos(lat),
        std::sin(lat) * std::sin(lon),
      };
      // Rotate local Z onto the surface normal, so the flat axis faces outwards. At the poles the
      // normal is already along Z and the rotation axis degenerates to zero: use identity.
      const auto axis = cross(Vec3{ 0.F, 0.F, 1.F }, normal);
      const auto axis_len = norm(axis);
      const auto angle = std::acos(normal[2]);
      const auto rotation =
          (axis_len > 0.F) ?
              grape::linalg::Quaternionf::fromAxisAngle(axis * (1.F / axis_len), angle) :
              Quat{};
      const auto hue = static_cast<float>(i) / static_cast<float>(NUM_LAT);
      const auto tangent_scale = 0.6F * RADIUS * pi_v<float> / static_cast<float>(NUM_LAT);
      splats.push_back({
          .position = RADIUS * normal,
          .scale = Vec3{ tangent_scale, tangent_scale, 0.005F },
          .rotation = rotation,
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
  constexpr auto IDENTITY_ROTATION = Quat{};
  for (auto k = 0; k < NUM_RING; ++k) {
    const auto theta = 2.F * pi_v<float> * static_cast<float>(k) / static_cast<float>(NUM_RING);
    splats.push_back({
        .position = Vec3{ 1.6F * std::cos(theta), -1.F, 1.6F * std::sin(theta) },
        .scale = Vec3{ 0.05F, 0.05F, 0.05F },
        .rotation = IDENTITY_ROTATION,
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
        { .title = "Splat viewer example", .up = grape::splat::Vec3{ 0.F, 1.F, 0.F } });
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
