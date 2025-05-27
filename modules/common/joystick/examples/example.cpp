//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <csignal>
#include <print>

#include "grape/exception.h"
#include "grape/joystick/joystick.h"

namespace {

//-------------------------------------------------------------------------------------------------
void printDeviceInfo(const std::vector<grape::joystick::DeviceInfo>& devices) {
  std::println("Devices available: {}", devices.size());
  for (const auto& device : devices) {
    std::println("{}\n\tpath: {}\n\tbus type: 0x{:04x} product ID: 0x{:04x} vendor ID: 0x{:04x} "
                 "hw version: 0x{:04x}",
                 device.name, device.path.string(), device.bus_type, device.product_id,
                 device.vendor_id, device.hw_version);
  }
}

//-------------------------------------------------------------------------------------------------
struct VisitJoystickEvent {
  void operator()(const grape::joystick::ConnectionEvent& ev) {
    std::println("{}", ev.is_connected ? "Connected" : "Disconnected");
  }
  void operator()(const grape::joystick::ErrorEvent& ev) {
    std::println("{}", ev.message);
  }
  void operator()(const grape::joystick::ControlEvent& ev) {
    std::println("{}\t{}\t{}\t{}", ev.timestamp, grape::joystick::toString(ev.type),
                 grape::joystick::toString(ev.id), ev.value);
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
// - Prints its capabilities
// - Reports events from the device until ctrl-c is pressed
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

    auto js = grape::joystick::Joystick(devices.at(0).path, callback);
    s_exit.wait(false);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}
