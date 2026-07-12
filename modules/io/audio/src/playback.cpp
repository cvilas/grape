//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/audio/playback.h"

#include <cstddef>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "grape/audio/audio_spec.h"
#include "grape/exception.h"
#include "grape/log/syslog.h"
#include "sdl_conversions.h"

namespace grape::audio {

//=================================================================================================
struct Playback::Impl {
  explicit Impl(const Playback::Config& cfg);
  void reconfigure(const AudioSpec& spec);

  SDL_AudioDeviceID device_id{};
  AudioSpec current_spec;
  detail::AudioStreamPtr output_stream;
};

//-------------------------------------------------------------------------------------------------
Playback::Impl::Impl(const Playback::Config& cfg)
  : device_id(cfg.device_id), output_stream(detail::makeAudioStreamPtr(nullptr)) {
}

//-------------------------------------------------------------------------------------------------
void Playback::Impl::reconfigure(const AudioSpec& spec) {
  const auto needs_reconfigure = (output_stream == nullptr) or (current_spec != spec);
  if (not needs_reconfigure) {
    return;
  }

  const auto desired_audio_spec = detail::toSdlAudioSpec(spec);
  auto stream = detail::makeAudioStreamPtr(
      SDL_OpenAudioDeviceStream(device_id, &desired_audio_spec, nullptr, nullptr));
  if (stream == nullptr) {
    panic(std::format("Failed to open output stream: {}", SDL_GetError()));
  }

  if (!SDL_ResumeAudioStreamDevice(stream.get())) {
    panic(std::format("Failed to start output stream: {}", SDL_GetError()));
  }

  output_stream = std::move(stream);
  current_spec = spec;
}

//-------------------------------------------------------------------------------------------------
Playback::Playback(const Config& config) : impl_(std::make_unique<Impl>(config)) {
}

//-------------------------------------------------------------------------------------------------
Playback::~Playback() = default;

//-------------------------------------------------------------------------------------------------
void Playback::play(const AudioFrame& frame) {
  if (frame.payload.empty()) {
    return;
  }
  impl_->reconfigure(frame.header.audio_spec);

  (void)SDL_PutAudioStreamData(impl_->output_stream.get(), frame.payload.data(),
                               static_cast<int>(frame.payload.size()));
}

}  // namespace grape::audio
