//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/splat/depth_sort.h"

#include <algorithm>
#include <cstddef>
#include <limits>

namespace grape::splat {

//-------------------------------------------------------------------------------------------------
auto sortBackToFront(std::span<const Splat> splats, const Mat4& view)
    -> std::vector<std::uint32_t> {
  static constexpr auto NUM_BUCKETS = std::size_t{ 1 } << 16U;

  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
  // Unchecked access in hot loops. Indices are bounded by construction.

  // View-space depth (distance along viewing direction) of each splat. Only the third row of the
  // view transform is needed: depth = -(row·p + offset), since the camera looks along -Z.
  const auto row = Vec3{ .x = view.at(2, 0), .y = view.at(2, 1), .z = view.at(2, 2) };
  const auto offset = view.at(2, 3);
  auto depths = std::vector<float>(splats.size());
  auto min_depth = std::numeric_limits<float>::max();
  auto max_depth = 0.F;
  for (auto i = 0UZ; i < splats.size(); ++i) {
    const auto depth = -(dot(row, splats[i].position) + offset);
    depths[i] = depth;
    if (depth > 0.F) {
      min_depth = std::min(min_depth, depth);
      max_depth = std::max(max_depth, depth);
    }
  }
  if (max_depth <= 0.F) {
    return {};
  }

  // Counting sort on quantised depth, with farthest splats in bucket 0:
  // 1. Count how many splats fall in each bucket
  // 2. Prefix sum: counts[b] becomes the index in the output where bucket b starts
  // 3. Place each splat at its bucket's next free slot
  const auto range = std::max(max_depth - min_depth, std::numeric_limits<float>::min());
  const auto scale = static_cast<float>(NUM_BUCKETS - 1) / range;
  const auto bucket_of = [&](float depth) {
    return static_cast<std::size_t>((max_depth - depth) * scale);
  };
  auto counts = std::vector<std::uint32_t>(NUM_BUCKETS + 1, 0);
  auto visible = std::uint32_t{ 0 };
  for (const auto depth : depths) {
    if (depth > 0.F) {
      ++counts[bucket_of(depth) + 1];
      ++visible;
    }
  }
  for (auto bucket = 1UZ; bucket <= NUM_BUCKETS; ++bucket) {
    counts[bucket] += counts[bucket - 1];
  }
  auto order = std::vector<std::uint32_t>(visible);
  for (auto i = 0UZ; i < depths.size(); ++i) {
    if (depths[i] > 0.F) {
      order[counts[bucket_of(depths[i])]++] = static_cast<std::uint32_t>(i);
    }
  }
  return order;
  // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
}

}  // namespace grape::splat
