//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/audio/recorder.h"

#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL_audio.h>

#include "grape/audio/audio_spec.h"
#include "grape/exception.h"
#include "grape/log/syslog.h"
#include "sdl_conversions.h"

namespace grape::audio {

//-------------------------------------------------------------------------------------------------
struct Recorder::Impl {
  explicit Impl(const Recorder::Config& cfg, Recorder::Callback&& cb);
  void recordCallback(SDL_AudioStream* input_stream_ref, int total_amount);

  std::uint64_t sequence_number = 0;
  AudioSpec audio_spec;
  Recorder::Callback callback;
  detail::AudioStreamPtr input_stream;
};

//-------------------------------------------------------------------------------------------------
Recorder::Impl::Impl(const Recorder::Config& cfg, Recorder::Callback&& cb)
  : audio_spec(cfg.audio_spec)
  , callback(std::move(cb))
  , input_stream(detail::makeAudioStreamPtr(nullptr)) {
  const auto desired_spec = detail::toSdlAudioSpec(audio_spec);

  input_stream = detail::makeAudioStreamPtr(SDL_OpenAudioDeviceStream(
      cfg.device_id, &desired_spec,
      [](void* userdata, SDL_AudioStream* stream, int /*additional_amount*/, int total_amount) {
        static_cast<Impl*>(userdata)->recordCallback(stream, total_amount);
      },
      this));
  if (input_stream == nullptr) {
    panic(std::format("Failed to open input stream: {}", SDL_GetError()));
  }

  if (!SDL_ResumeAudioStreamDevice(input_stream.get())) {
    panic(std::format("Failed to start input stream: {}", SDL_GetError()));
  }
}

//-------------------------------------------------------------------------------------------------
void Recorder::Impl::recordCallback(SDL_AudioStream* input_stream_ref, int total_amount) {
  if (total_amount <= 0) {
    return;
  }

  auto payload = std::vector<std::byte>(static_cast<std::size_t>(total_amount));
  const int captured = SDL_GetAudioStreamData(input_stream_ref, payload.data(), total_amount);
  if ((captured <= 0) || (callback == nullptr)) {
    return;
  }
  payload.resize(static_cast<std::size_t>(captured));

  const auto bytes_per_sample = detail::bytesPerSample(audio_spec.sample_format);

  auto frame = AudioFrame{
      .header = {
        .audio_spec = audio_spec,
        .sequence_number = sequence_number++,
        .samples_per_channel =
            static_cast<std::uint32_t>(captured / static_cast<int>(bytes_per_sample * audio_spec.channels)),
      },
      .payload = std::span<const std::byte>{ payload.data(), payload.size() },
    };

  callback(frame);
}

//-------------------------------------------------------------------------------------------------
Recorder::Recorder(const Config& config, Callback&& callback)
  : impl_(std::make_unique<Impl>(config, std::move(callback))) {
}

//-------------------------------------------------------------------------------------------------
Recorder::~Recorder() = default;

}  // namespace grape::audio
