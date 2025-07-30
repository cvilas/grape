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
  ImageFrame image;
  const auto header_size = sizeof(image.header);
  if (bytes.size_bytes() < header_size) {
    syslog::Error("Input data too small for header");
    return false;
  }
  std::memcpy(&image.header, bytes.data(), header_size);

  const auto data_size = image.header.pitch * image.header.height;
  if (decompress_buffer_.size() < data_size) {
    decompress_buffer_.resize(data_size);
  }

  const auto decompressed_data_size = LZ4_decompress_safe(
      reinterpret_cast<const char*>(bytes.data() + header_size),
      reinterpret_cast<char*>(decompress_buffer_.data()),
      static_cast<int>(bytes.size_bytes() - header_size), static_cast<int>(data_size));

  if (std::cmp_not_equal(decompressed_data_size, data_size)) {
    syslog::Error("Failed to decompress image data");
    return false;
  }
  image.pixels = std::span{ decompress_buffer_ };

  if (callback_) {
    callback_(image);
  }
  return true;
}

}  // namespace grape::camera
