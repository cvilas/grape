//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cstdint>
#include <vector>

#include "catch2/catch_test_macros.hpp"
#include "grape/splat/depth_sort.h"
#include "grape/splat/examiner_camera.h"
#include "grape/splat/splat.h"

namespace {

using grape::splat::ExaminerCamera;
using grape::splat::ORIGIN;
using grape::splat::Splat;
using grape::splat::Vec3;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
auto splatAt(const Vec3& position) -> Splat {
  auto splat = Splat{};
  splat.position = position;
  return splat;
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Splats are ordered farthest first", "[depth_sort]") {
  // Identity view: camera at origin looking along -Z
  const auto splats = std::vector<Splat>{
    splatAt({ .z = -2.F }),
    splatAt({ .z = -10.F }),
    splatAt({ .x = 3.F, .z = -5.F }),
    splatAt({ .z = -1.F }),
  };
  CHECK(sortBackToFront(splats, {}) == std::vector<std::uint32_t>{ 1, 2, 0, 3 });
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Splats behind the camera are culled", "[depth_sort]") {
  const auto splats = std::vector<Splat>{
    splatAt({ .z = 2.F }),
    splatAt({ .z = -3.F }),
    splatAt({ .z = 0.F }),
  };
  CHECK(sortBackToFront(splats, {}) == std::vector<std::uint32_t>{ 1 });
  CHECK(sortBackToFront(std::vector<Splat>{ splatAt({ .z = 1.F }) }, {}).empty());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Sort order follows the camera", "[depth_sort]") {
  const auto splats = std::vector<Splat>{
    splatAt({ .z = 1.F }),
    splatAt({ .z = -1.F }),
  };
  auto camera = ExaminerCamera{};

  camera.viewAll(ORIGIN, 1.F, { .y = 1.F });  // camera on +Z axis
  CHECK(sortBackToFront(splats, camera.viewMatrix()) == std::vector<std::uint32_t>{ 1, 0 });

  camera.viewAll(ORIGIN, 1.F, { .y = -1.F });  // camera on -Z axis
  CHECK(sortBackToFront(splats, camera.viewMatrix()) == std::vector<std::uint32_t>{ 0, 1 });
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace
