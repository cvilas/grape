//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include "grape/splat/splat.h"

namespace grape::splat {

/// Size of one record in a `.splat` file (bytes)
inline constexpr auto SPLAT_RECORD_SIZE = 32U;

/// Decode the contents of a `.splat` file. See docs/file_formats.md
/// @param data File contents
/// @return Decoded splats, or a description of the error
[[nodiscard]] auto decodeSplat(std::span<const std::byte> data)
    -> std::expected<std::vector<Splat>, std::string>;

/// Encode splats into `.splat` file contents. See docs/file_formats.md
/// @note Lossy: colour, opacity and rotation are quantised to 8 bits per component
[[nodiscard]] auto encodeSplat(std::span<const Splat> splats) -> std::vector<std::byte>;

/// Decode the contents of a binary little-endian PLY file produced by 3D gaussian splatting
/// training pipelines (Kerbl et al., 2023). See docs/file_formats.md
/// @note Only the view-independent colour (f_dc_*) is used; higher-order SH (f_rest_*) is ignored
/// @param data File contents
/// @return Decoded splats, or a description of the error
[[nodiscard]] auto decodePly(std::span<const std::byte> data)
    -> std::expected<std::vector<Splat>, std::string>;

/// Load splats from file. Format is selected by file extension (`.splat` or `.ply`)
/// @param path Path to file
/// @return Loaded splats, or a description of the error
[[nodiscard]] auto load(const std::filesystem::path& path)
    -> std::expected<std::vector<Splat>, std::string>;

/// Save splats to file in `.splat` format
/// @param path Path to file
/// @param splats Splats to save
/// @return Nothing on success, or a description of the error
[[nodiscard]] auto save(const std::filesystem::path& path, std::span<const Splat> splats)
    -> std::expected<void, std::string>;

}  // namespace grape::splat
