//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "grape/splat/math.h"

namespace grape::splat {

//=================================================================================================
/// 8-bit per channel colour. RGB are display (sRGB) values; alpha is opacity.
struct Rgba {
  std::uint8_t r{ 0 };
  std::uint8_t g{ 0 };
  std::uint8_t b{ 0 };
  std::uint8_t a{ 0 };

  constexpr auto operator==(const Rgba&) const -> bool = default;
};

//=================================================================================================
/// A single 3D gaussian: a soft, coloured, semi-transparent ellipsoid. A scene is a list of these,
/// typically between 100 thousand and a few million. See docs/gaussian_splatting.md.
///
/// Mathematically, it is the function G(p) = exp(-½·(p - μ)ᵀ·Σ⁻¹·(p - μ)) scaled by an opacity,
/// where μ is `position` and Σ is the 3x3 covariance matrix (see covariance()). G is 1 at the
/// centre and falls off smoothly, faster along the short axes of the ellipsoid.
///
/// The covariance is not stored directly. Instead, it is defined by `scale` and `rotation`:
/// Σ = R·S·Sᵀ·Rᵀ, where R is the rotation matrix of `rotation` and S = diag(`scale`). These are
/// intuitive (an ellipsoid's size and orientation) and always yield a valid covariance.
///
/// Only the view-independent colour is represented. Full gaussian splatting scenes also store
/// colour that changes with viewing direction (spherical harmonics), for shiny surfaces.
struct Splat {
  Vec3 position;  //!< Centre μ (metres)
  Vec3 scale;     //!< Standard deviation (1σ 'radius') along each local axis (metres, > 0)
  Quat rotation;  //!< Orientation of the local axes (unit quaternion)
  Rgba color;     //!< Colour, and opacity at the centre (alpha)
};

//=================================================================================================
/// Upper triangle of a symmetric 3x3 covariance matrix: {xx, xy, xz, yy, yz, zz}
///
/// The covariance generalises variance (σ²) to 3D. Diagonal terms are the variances along x, y and
/// z; off-diagonal terms say how much the ellipsoid is tilted between those axes.
using Covariance = std::array<float, 6>;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)

/// @return 3D covariance matrix Σ = R·S·Sᵀ·Rᵀ of the gaussian
[[nodiscard]] auto covariance(const Splat& splat) -> Covariance;

//=================================================================================================
/// A bounding sphere
struct Sphere {
  Vec3 center;
  float radius{ 0.F };
};

/// Default fraction of splats enclosed by computeBounds(). The median frames the subject of typical
/// captured scenes, where a large fraction of splats is distant background.
static constexpr auto DEFAULT_BOUNDS_PERCENTILE = 0.5F;

/// Compute a bounding sphere for the main subject of a scene.
///
/// Captured scenes usually contain a sparse cloud of far-away 'background' gaussians. A
/// conventional bounding sphere would therefore be too large to frame the subject. Instead, the
/// centre is the per-axis median of positions and the radius is the given percentile of distances
/// from that centre.
/// @param splats The scene
/// @param percentile Fraction of splats in [0, 1] to enclose in the sphere
/// @return Bounding sphere. Zero radius for empty scenes.
[[nodiscard]] auto computeBounds(std::span<const Splat> splats,
                                 float percentile = DEFAULT_BOUNDS_PERCENTILE) -> Sphere;

}  // namespace grape::splat
