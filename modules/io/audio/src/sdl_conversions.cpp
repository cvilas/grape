//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "sdl_conversions.h"

#include <cstdint>
#include <format>
#include <span>

#include "grape/exception.h"

namespace grape::audio::detail {

//-------------------------------------------------------------------------------------------------
auto makeAudioStreamPtr(SDL_AudioStream* stream) -> AudioStreamPtr {
  return AudioStreamPtr{ stream, SDL_DestroyAudioStream };
}

//-------------------------------------------------------------------------------------------------
auto toSdlAudioFormat(SampleFormat sample_format) -> SDL_AudioFormat {
  switch (sample_format) {
    case SampleFormat::PcmU8:
      return SDL_AUDIO_U8;
    case SampleFormat::PcmS16Le:
      return SDL_AUDIO_S16LE;
    case SampleFormat::PcmS32Le:
      return SDL_AUDIO_S32LE;
    case SampleFormat::PcmF32Le:
      return SDL_AUDIO_F32LE;
    case SampleFormat::Unknown:
      break;
  }

  grape::panic(
      std::format("Unsupported sample format: {}", static_cast<std::uint16_t>(sample_format)));
}

//-------------------------------------------------------------------------------------------------
auto toSampleFormat(SDL_AudioFormat sample_format) -> SampleFormat {
  switch (sample_format) {
    case SDL_AUDIO_U8:
      return SampleFormat::PcmU8;
    case SDL_AUDIO_S16LE:
      return SampleFormat::PcmS16Le;
    case SDL_AUDIO_S32LE:
      return SampleFormat::PcmS32Le;
    case SDL_AUDIO_F32LE:
      return SampleFormat::PcmF32Le;
    case SDL_AUDIO_UNKNOWN:
      [[fallthrough]];
    case SDL_AUDIO_S8:
      [[fallthrough]];
    case SDL_AUDIO_S16BE:
      [[fallthrough]];
    case SDL_AUDIO_S32BE:
      [[fallthrough]];
    case SDL_AUDIO_F32BE:
      break;
  }

  grape::panic(
      std::format("Unsupported SDL sample format: {}", static_cast<std::uint16_t>(sample_format)));
}

//-------------------------------------------------------------------------------------------------
auto toSdlAudioSpec(const AudioSpec& spec) -> SDL_AudioSpec {
  return SDL_AudioSpec{ .format = toSdlAudioFormat(spec.sample_format),
                        .channels = static_cast<int>(spec.channels),
                        .freq = static_cast<int>(spec.sample_rate_hz) };
}

//-------------------------------------------------------------------------------------------------
auto bytesPerSample(SampleFormat sample_format) -> std::uint16_t {
  switch (sample_format) {
    case SampleFormat::PcmU8:
      return 1U;
    case SampleFormat::PcmS16Le:
      return 2U;
    case SampleFormat::PcmS32Le:
    case SampleFormat::PcmF32Le:
      return 4U;
    case SampleFormat::Unknown:
      break;
  }

  grape::panic(
      std::format("Unsupported sample format: {}", static_cast<std::uint16_t>(sample_format)));
}

}  // namespace grape::audio::detail
