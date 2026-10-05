//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "grape/splat/math.h"
#include "grape/splat/splat.h"

namespace grape::splat {

/// Order splats for back-to-front alpha blending.
///
/// Why sort? Splats are semi-transparent. Blending a splat over the image computes
/// `new = splat·α + (1 - α)·old`, which is only correct if everything behind the splat is already
/// in `old`. So splats must be drawn farthest first. Sorting once per splat (by centre depth)
/// rather than per pixel is the key approximation that makes gaussian splatting fast. It is
/// occasionally wrong where splats intersect, which shows up as brief 'popping' as the view moves.
///
/// How: The depth range is divided into 65536 buckets and splats are distributed into them with a
/// counting sort. This is O(N), much faster than a comparison sort for millions of splats, and the
/// rounding error (range/65536) is too small to matter. Splats behind the camera are dropped.
/// @param splats The scene
/// @param view World-to-camera transform (camera looks along its -Z axis)
/// @return Indices into `splats`, farthest first
[[nodiscard]] auto sortBackToFront(std::span<const Splat> splats, const Mat4& view)
    -> std::vector<std::uint32_t>;

}  // namespace grape::splat
