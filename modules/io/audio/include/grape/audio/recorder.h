//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "grape/audio/audio_frame.h"
#include "grape/audio/audio_spec.h"

namespace grape::audio {

//=================================================================================================
class Recorder {
public:
  using Callback = std::function<void(const AudioFrame&)>;

  struct Config {
    static constexpr auto DEFAULT_SAMPLE_RATE_HZ = 48000;
    std::uint32_t device_id;
    AudioSpec audio_spec{ .channels = 1,
                          .sample_format = SampleFormat::PcmF32Le,
                          .sample_rate_hz = DEFAULT_SAMPLE_RATE_HZ };
  };

  Recorder(const Config& config, Callback&& callback);

  ~Recorder();
  Recorder(const Recorder&) = delete;
  Recorder(Recorder&&) = delete;
  auto operator=(const Recorder&) -> Recorder& = delete;
  auto operator=(Recorder&&) -> Recorder& = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::audio
