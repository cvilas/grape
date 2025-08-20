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
  static constexpr auto HDR_SIZE = sizeof(ImageFrame::Header);
  const auto data_size = image.pixels.size_bytes();

  const auto max_compressed_data_size = LZ4_compressBound(static_cast<int>(data_size));

  const auto max_dst_size = HDR_SIZE + static_cast<std::size_t>(max_compressed_data_size);
  if (buffer_.size() < max_dst_size) {
    buffer_.resize(max_dst_size);
  }

  std::memcpy(buffer_.data(), &(image.header), HDR_SIZE);

  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  static constexpr auto LZ4_ACC = 100;  // 1 = max compression, 65537 = max speed; see lz4 docs
  const auto compressed_data_size =
      LZ4_compress_fast(reinterpret_cast<const char*>(image.pixels.data()),
                        reinterpret_cast<char*>(std::next(buffer_.data(), HDR_SIZE)),
                        static_cast<int>(data_size), max_compressed_data_size, LZ4_ACC);
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
  if (compressed_data_size <= 0) {
    syslog::Error("Failed to compress image data");
    return false;
  }
  const auto src_size = data_size + HDR_SIZE;
  const auto dst_size = static_cast<std::size_t>(compressed_data_size) + HDR_SIZE;
  callback_({ buffer_.data(), dst_size }, { .src_size = src_size, .dst_size = dst_size });

  return true;
}

}  // namespace grape::camera
