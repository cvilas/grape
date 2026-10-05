//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/splat/splat.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace grape::splat {

//-------------------------------------------------------------------------------------------------
auto covariance(const Splat& splat) -> Covariance {
  // Σ = R·S·Sᵀ·Rᵀ = M·Mᵀ, with M = R·S.
  //
  // Read right to left, this says: start with a unit sphere, stretch it along the local axes by
  // the standard deviations in S, then rotate it by R. The product of a matrix with its own
  // transpose is always symmetric and positive semi-definite, which is exactly what a covariance
  // must be. That is why splats store (scale, rotation) rather than Σ directly.
  const auto rot = toMatrix(normalize(splat.rotation));
  const auto scale = std::array{ splat.scale.x, splat.scale.y, splat.scale.z };

  // M = R·S: S is diagonal, so this scales column j of R by scale[j]
  auto mat = Mat3{};
  for (auto row = 0U; row < 3U; ++row) {
    for (auto col = 0U; col < 3U; ++col) {
      mat.at(row).at(col) = rot.at(row).at(col) * scale.at(col);
    }
  }

  // Σ[i][j] = Σ_k M[i][k]·M[j][k], i.e. the dot product of rows i and j of M
  const auto row_of = [&mat](std::size_t idx) {
    return Vec3{ .x = mat.at(idx).at(0), .y = mat.at(idx).at(1), .z = mat.at(idx).at(2) };
  };
  const auto sigma = [&row_of](std::size_t row, std::size_t col) {
    return dot(row_of(row), row_of(col));
  };
  return { sigma(0, 0), sigma(0, 1), sigma(0, 2), sigma(1, 1), sigma(1, 2), sigma(2, 2) };
}

//-------------------------------------------------------------------------------------------------
auto computeBounds(std::span<const Splat> splats, float percentile) -> Sphere {
  if (splats.empty()) {
    return {};
  }
  const auto count = splats.size();
  const auto mid = count / 2;
  const auto median = [&splats, count, mid](auto&& component) {
    auto values = std::vector<float>(count);
    std::ranges::transform(splats, values.begin(), component);
    std::ranges::nth_element(values, values.begin() + static_cast<std::ptrdiff_t>(mid));
    return values.at(mid);
  };
  const auto center = Vec3{
    .x = median([](const Splat& splat) { return splat.position.x; }),
    .y = median([](const Splat& splat) { return splat.position.y; }),
    .z = median([](const Splat& splat) { return splat.position.z; }),
  };

  auto distances = std::vector<float>(count);
  std::ranges::transform(splats, distances.begin(),
                         [&center](const Splat& splat) { return length(splat.position - center); });
  const auto frac = std::clamp(percentile, 0.F, 1.F);
  const auto nth = std::min(count - 1, static_cast<std::size_t>(frac * static_cast<float>(count)));
  std::ranges::nth_element(distances, distances.begin() + static_cast<std::ptrdiff_t>(nth));
  return { .center = center, .radius = distances.at(nth) };
}

}  // namespace grape::splat
