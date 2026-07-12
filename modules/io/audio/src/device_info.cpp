//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/audio/device_info.h"

#include <span>

#include <SDL3/SDL_audio.h>

namespace grape::audio {

//-------------------------------------------------------------------------------------------------
auto enumerate(Direction dir) -> std::vector<DeviceInfo> {
  int count = 0;
  auto* device_ids = (dir == Direction::Input) ? SDL_GetAudioRecordingDevices(&count) :
                                                 SDL_GetAudioPlaybackDevices(&count);
  auto devices = std::vector<DeviceInfo>{};
  for (const auto id : std::span{ device_ids, static_cast<std::size_t>(count) }) {
    if (const auto* name = SDL_GetAudioDeviceName(id); name != nullptr) {
      devices.emplace_back(id, name);
    }
  }
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,bugprone-multi-level-implicit-pointer-conversion)
  SDL_free(device_ids);
  return devices;
}

}  // namespace grape::audio
