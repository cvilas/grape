//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "grape/audio/audio_spec.h"

namespace grape::audio {

//=================================================================================================
/// Single audio frame/chunk payload and metadata
struct AudioFrame {
  struct Header {
    AudioSpec audio_spec{};
    std::uint64_t sequence_number = 0;
    std::uint32_t samples_per_channel = 0;
  };

  Header header;
  std::span<const std::byte> payload;
};

}  // namespace grape::audio
