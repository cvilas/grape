//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

#include "grape/audio/audio_spec.h"
#include "grape/audio/device_info.h"

namespace grape::audio::detail {

using AudioStreamPtr = std::unique_ptr<SDL_AudioStream, decltype(&SDL_DestroyAudioStream)>;

[[nodiscard]] auto makeAudioStreamPtr(SDL_AudioStream* stream) -> AudioStreamPtr;
[[nodiscard]] auto toSdlAudioFormat(SampleFormat sample_format) -> SDL_AudioFormat;
[[nodiscard]] auto toSampleFormat(SDL_AudioFormat sample_format) -> SampleFormat;
[[nodiscard]] auto toSdlAudioSpec(const AudioSpec& spec) -> SDL_AudioSpec;
[[nodiscard]] auto bytesPerSample(SampleFormat sample_format) -> std::uint16_t;

}  // namespace grape::audio::detail
