//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <chrono>
#include <csignal>
#include <print>
#include <thread>

#include "grape/exception.h"
#include "grape/joystick/joystick.h"

namespace {

//-------------------------------------------------------------------------------------------------
void printDeviceInfo(const std::vector<std::filesystem::path>& paths) {
  std::println("Devices available: {}", paths.size());
  for (const auto& path : paths) {
    const auto info = grape::joystick::readDeviceInfo(path);
    if (info) {
      std::println("{}\n\tpath: {}\n\tbus type: 0x{:04x} product ID: 0x{:04x} vendor ID: 0x{:04x} "
                   "hw version: 0x{:04x}",
                   info->name, info->path.string(), info->bus_type, info->product_id,
                   info->vendor_id, info->hw_version);
    }
  }
}

//-------------------------------------------------------------------------------------------------
struct VisitJoystickEvent {
  void operator()(const grape::joystick::ConnectionEvent& ev) {
    std::println("[{}] {}", ev.timestamp, ev.is_connected ? "Connected" : "Disconnected");
  }
  void operator()(const grape::joystick::ErrorEvent& ev) {
    std::println("[{}] {}", ev.timestamp, ev.message);
  }
  void operator()(const grape::joystick::ButtonEvent& ev) {
    std::println("[{}] {}\t{}", ev.timestamp, toString(ev.id), ev.pressed ? "Pressed" : "Released");
  }
  void operator()(const grape::joystick::AxisEvent& ev) {
    std::println("[{}] {}\t{}", ev.timestamp, toString(ev.id), ev.value);
  }
};

//-------------------------------------------------------------------------------------------------
void callback(const grape::joystick::Event& ev) {
  try {
    std::visit(VisitJoystickEvent(), ev);
  } catch (const std::exception& ex) {
    std::println(stderr, "{}", ex.what());
  }
}

//-------------------------------------------------------------------------------------------------
std::atomic_flag s_exit = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void onSignal(int /*signum*/) {
  s_exit.test_and_set();
  s_exit.notify_one();
}

}  // namespace

//=================================================================================================
// Demonstrative example for joysticks interface
// - Enumerates available joysticks
// - Connects to the first one
// - Reports events from the device until ctrl-c is pressed
// - Automatically reconnects on device error or disconnection
auto main() -> int {
  try {
    std::ignore = signal(SIGINT, onSignal);
    std::ignore = signal(SIGTERM, onSignal);

    const auto devices = grape::joystick::enumerate();
    if (devices.empty()) {
      std::println("No joysticks available");
      return EXIT_SUCCESS;
    }
    printDeviceInfo(devices);

    auto js = grape::joystick::Joystick(callback);

    static constexpr auto EVENT_WAIT_TIMEOUT = std::chrono::milliseconds(100);
    static constexpr auto RECONNECTION_DELAY = std::chrono::milliseconds(1000);
    bool is_connected = js.open(devices.at(0));
    while (not s_exit.test()) {
      if (not is_connected) {
        // attempt reconnection
        std::this_thread::sleep_for(RECONNECTION_DELAY);
        is_connected = js.open(devices.at(0));
        continue;
      }
      // wait for events
      is_connected = js.wait(EVENT_WAIT_TIMEOUT);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
