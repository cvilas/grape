//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/decompressor.h"

#include <utility>

#include <lz4.h>

#include "grape/log/syslog.h"

namespace grape::camera {

//-------------------------------------------------------------------------------------------------
Decompressor::Decompressor(Callback&& cb) : callback_(std::move(cb)) {
}

//-------------------------------------------------------------------------------------------------
auto Decompressor::decompress(std::span<const std::byte> bytes) -> bool {
  static constexpr auto HDR_SIZE = sizeof(ImageFrame::Header);
  if (bytes.size_bytes() < HDR_SIZE) {
    syslog::Error("Input data too small for header");
    return false;
  }

  ImageFrame image;
  std::memcpy(&image.header, bytes.data(), HDR_SIZE);

  const auto data_size = image.header.width * image.header.height * 3U;  // enough for up to RGB24
  if (buffer_.size() < data_size) {
    buffer_.resize(data_size);
  }

  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto decompressed_data_size = LZ4_decompress_safe(
      reinterpret_cast<const char*>(&bytes[HDR_SIZE]), reinterpret_cast<char*>(buffer_.data()),
      static_cast<int>(bytes.size_bytes() - HDR_SIZE), static_cast<int>(data_size));
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
  if (decompressed_data_size <= 0) {
    syslog::Error("Failed to decompress image data");
    return false;
  }
  image.pixels = std::span{ buffer_.data(), static_cast<std::size_t>(decompressed_data_size) };

  callback_(image);

  return true;
}

}  // namespace grape::camera
