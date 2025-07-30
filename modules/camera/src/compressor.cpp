//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/camera/compressor.h"

#include <cstring>  // memcpy

#include <lz4.h>

#include "grape/log/syslog.h"

namespace grape::camera {

//-------------------------------------------------------------------------------------------------
Compressor::Compressor(Callback&& cb) : callback_(std::move(cb)) {
}

//-------------------------------------------------------------------------------------------------
auto Compressor::compress(const ImageFrame& image) -> bool {
  const auto header_size = sizeof(image.header);
  const auto data_size = static_cast<int>(image.pixels.size_bytes());

  const auto max_compressed_data_size = LZ4_compressBound(data_size);

  const auto max_dst_size = header_size + static_cast<std::size_t>(max_compressed_data_size);
  if (compress_buffer_.size() < max_dst_size) {
    compress_buffer_.resize(max_dst_size);
  }

  // Write metadata
  std::memcpy(compress_buffer_.data(), &(image.header), header_size);

  const auto compressed_data_size =
      LZ4_compress_default(reinterpret_cast<const char*>(image.pixels.data()),
                           reinterpret_cast<char*>(compress_buffer_.data() + header_size),
                           data_size, max_compressed_data_size);
  if (compressed_data_size <= 0) {
    syslog::Error("Failed to compress image data");
    return false;
  }

  if (callback_) {
    callback_(
        { compress_buffer_.data(), static_cast<std::size_t>(compressed_data_size) + header_size });
  }

  return true;
}

}  // namespace grape::camera
