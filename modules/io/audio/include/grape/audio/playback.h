//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "grape/audio/audio_frame.h"

namespace grape::audio {

//=================================================================================================
class Playback {
public:
  struct Config {
    std::uint32_t device_id;
  };

  explicit Playback(const Config& config);

  void play(const AudioFrame& frame);

  ~Playback();
  Playback(const Playback&) = delete;
  Playback(Playback&&) = delete;
  auto operator=(const Playback&) -> Playback& = delete;
  auto operator=(Playback&&) -> Playback& = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::audio
