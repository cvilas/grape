//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#define SDL_MAIN_USE_CALLBACKS 1  // NOLINT(cppcoreguidelines-macro-usage)

#include <format>
#include <memory>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "grape/audio/device_info.h"
#include "grape/audio/playback.h"
#include "grape/audio/recorder.h"
#include "grape/conio/program_options.h"
#include "grape/exception.h"
#include "grape/log/config.h"
#include "grape/log/record.h"
#include "grape/log/severity.h"
#include "grape/log/sinks/console_sink.h"
#include "grape/log/syslog.h"

//-------------------------------------------------------------------------------------------------
// Demonstrates using SDL3 audio API to capture and render audio data.
//-------------------------------------------------------------------------------------------------

namespace {

//=================================================================================================
/// Encapsulates processing pipeline
class Application {
public:
  explicit Application(std::uint32_t input_device_id, std::uint32_t output_device_id);
  auto handleEvent(const SDL_Event& event) -> SDL_AppResult;

private:
  void onRecordedFrame(const grape::audio::AudioFrame& frame) const;

  std::unique_ptr<grape::audio::Playback> playback_;
  std::unique_ptr<grape::audio::Recorder> recorder_;
};

//-------------------------------------------------------------------------------------------------
struct LogFormatter {
  static auto format(const grape::log::Record& record) -> std::string {
    return std::format("[{}] [{:9s}] {}", record.timestamp, toString(record.severity),
                       record.message.cStr());
  }
};

//-------------------------------------------------------------------------------------------------
void setupLogging() {
  auto log_config = grape::log::Config{};
  log_config.sink = std::make_shared<grape::log::ConsoleSink<LogFormatter>>();
  log_config.threshold = grape::log::Severity::Info;
  grape::syslog::init(std::move(log_config));
}

Application::Application(std::uint32_t input_device_id, std::uint32_t output_device_id) {
  playback_ = std::make_unique<grape::audio::Playback>(
      grape::audio::Playback::Config{ .device_id = output_device_id });
  recorder_ = std::make_unique<grape::audio::Recorder>(
      grape::audio::Recorder::Config{ .device_id = input_device_id },
      [this](const grape::audio::AudioFrame& frame) { onRecordedFrame(frame); });
}

//-------------------------------------------------------------------------------------------------
void Application::onRecordedFrame(const grape::audio::AudioFrame& frame) const {
  playback_->play(frame);
}

//-------------------------------------------------------------------------------------------------
auto Application::handleEvent(const SDL_Event& event) -> SDL_AppResult {
  if (event.type == SDL_EVENT_QUIT) {
    grape::syslog::Info("Quit!");
    recorder_.reset();
    playback_.reset();
    return SDL_APP_SUCCESS;
  }
  // TODO: handle device removal
  return SDL_APP_CONTINUE;
}

}  // namespace

auto SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult {
  try {
    setupLogging();

    const auto args =
        grape::conio::ProgramDescription("Audio monitor application")
            .declareOption<std::string>("input", "Part of audio input device name to match", "")
            .declareOption<std::string>("output", "Part of audio output device name to match", "")
            .parse(argc, const_cast<const char**>(argv));

    if (not SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
      grape::syslog::Critical("SDL_Init failed: {}", SDL_GetError());
      return SDL_APP_FAILURE;
    }

    const auto capture_devices = grape::audio::enumerate(grape::audio::Direction::Input);
    grape::syslog::Info("Available capture devices:");
    for (const auto& dev : capture_devices) {
      grape::syslog::Info("  {}", dev.name);
    }

    const auto input_hint = args.get<std::string>("input");
    auto input_it = std::ranges::find_if(capture_devices, [&input_hint](const auto& info) {
      return info.name.contains(input_hint);
    });

    if (input_it == capture_devices.end()) {
      grape::syslog::Warn("No input device matching '{}'. Will open default", input_hint);
      input_it = capture_devices.begin();
    }
    grape::syslog::Note("Using input device: {}", input_it->name);

    const auto playback_devices = grape::audio::enumerate(grape::audio::Direction::Output);
    grape::syslog::Info("Available playback devices:");
    for (const auto& dev : playback_devices) {
      grape::syslog::Info("  {}", dev.name);
    }

    const auto output_hint = args.get<std::string>("output");
    auto output_it = std::ranges::find_if(playback_devices, [&output_hint](const auto& info) {
      return info.name.contains(output_hint);
    });

    if (output_it == playback_devices.end()) {
      grape::syslog::Warn("No output device matching '{}'. Will open default", output_hint);
      output_it = playback_devices.begin();
    }
    grape::syslog::Note("Using output device: {}", output_it->name);

    static auto app = std::make_unique<Application>(input_it->id, output_it->id);
    *appstate = app.get();
    return SDL_APP_CONTINUE;
  } catch (...) {
    grape::Exception::print();
    return SDL_APP_FAILURE;
  }
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppIterate(void* appstate) -> SDL_AppResult {
  (void)appstate;
  return SDL_APP_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
auto SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult {
  auto* app = static_cast<Application*>(appstate);
  return app->handleEvent(*event);
}

//-------------------------------------------------------------------------------------------------
void SDL_AppQuit(void* /*appstate*/, SDL_AppResult /*result*/) {
}
