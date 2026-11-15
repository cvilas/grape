//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/splat/splat.h"

#include <algorithm>
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
  const auto rot = rotationOf(normalize(splat.rotation));

  // M = R·S: S is diagonal, so this scales column j of R by scale[j]
  auto mat = rot;
  for (auto col = 0UZ; col < 3UZ; ++col) {
    for (auto row = 0UZ; row < 3UZ; ++row) {
      mat[row, col] *= splat.scale[col];
    }
  }

  const auto sigma = mat * linalg::transpose(mat);
  return { sigma[0, 0], sigma[0, 1], sigma[0, 2], sigma[1, 1], sigma[1, 2], sigma[2, 2] };
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
    median([](const Splat& splat) { return splat.position[0]; }),
    median([](const Splat& splat) { return splat.position[1]; }),
    median([](const Splat& splat) { return splat.position[2]; }),
  };

  auto distances = std::vector<float>(count);
  std::ranges::transform(splats, distances.begin(), [&center](const Splat& splat) {
    return linalg::norm(splat.position - center);
  });
  const auto frac = std::clamp(percentile, 0.F, 1.F);
  const auto nth = std::min(count - 1, static_cast<std::size_t>(frac * static_cast<float>(count)));
  std::ranges::nth_element(distances, distances.begin() + static_cast<std::ptrdiff_t>(nth));
  return { .center = center, .radius = distances.at(nth) };
}

}  // namespace grape::splat
