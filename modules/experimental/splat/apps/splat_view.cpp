//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <array>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <numbers>
#include <print>
#include <string>
#include <string_view>
#include <utility>

#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/splat/file_io.h"
#include "grape/splat/math.h"
#include "grape/splat/viewer.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto parseAxis(std::string_view name) -> grape::splat::Vec3 {
  using grape::splat::Vec3;
  static constexpr auto AXES = std::to_array<std::pair<std::string_view, Vec3>>({
      { "x", { .x = 1.F } },
      { "-x", { .x = -1.F } },
      { "y", { .y = 1.F } },
      { "-y", { .y = -1.F } },
      { "z", { .z = 1.F } },
      { "-z", { .z = -1.F } },
  });
  for (const auto& [key, axis] : AXES) {
    if (key == name) {
      return axis;
    }
  }
  grape::panic(std::format("Invalid axis '{}'. Must be one of x, -x, y, -y, z, -z", name));
}

}  // namespace

//=================================================================================================
// Interactive viewer for 3D gaussian splat files (.splat, .ply). See README.md for user controls.
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_FOV_DEG = 50.F;
    static constexpr auto HALF_TURN_DEG = 180.F;
    const auto args =
        grape::conio::ProgramDescription("Interactive 3D gaussian splat viewer")
            .declareOption<std::string>("file", "Splat file to view (.splat or .ply)")
            .declareOption<std::string>("up", "World axis shown as screen-up (x,-x,y,-y,z,-z)",
                                        "-y")
            .declareOption<float>("fov", "Vertical field of view (degrees)", DEFAULT_FOV_DEG)
            .parse(argc, argv);

    const auto path = std::filesystem::path(args.get<std::string>("file"));
    auto splats = grape::splat::load(path);
    if (not splats) {
      std::println(stderr, "{}", splats.error());
      return EXIT_FAILURE;
    }
    std::println("Loaded {} splats from {}", splats->size(), path.string());

    auto viewer = grape::splat::Viewer({
        .title = std::format("Splat Viewer - {}", path.filename().string()),
        .up = parseAxis(args.get<std::string>("up")),
        .fov_y = args.get<float>("fov") * std::numbers::pi_v<float> / HALF_TURN_DEG,
    });
    viewer.setScene(std::move(*splats));
    while (viewer.processEvents()) {
      viewer.render();
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
