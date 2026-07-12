//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <compare>
#include <cstdint>

namespace grape::audio {

// TODO
// - Consider making SampleFormat a struct with uint8_t endianness and uint8_t format
// - specify formats as Unknown, PcmF32, PcmS16, PcmS32, PcmU8
// - Specify endiannes separately in audiospec as a enum class Endianness:uint8_t { Little, Big}
enum class SampleFormat : std::uint16_t {
  Unknown = 0,
  PcmU8 = 1,
  PcmS16Le = 2,
  PcmS32Le = 3,
  PcmF32Le = 4,
};

//=================================================================================================
/// Audio stream specification
struct AudioSpec {
  std::uint16_t channels{};
  SampleFormat sample_format{ SampleFormat::Unknown };
  std::uint32_t sample_rate_hz{};

  constexpr auto operator<=>(const AudioSpec&) const = default;
};

}  // namespace grape::audio
