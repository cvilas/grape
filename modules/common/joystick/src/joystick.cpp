//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/joystick/joystick.h"

#include <atomic>
#include <concepts>  //std::invocable
#include <cstring>
#include <format>
#include <thread>
#include <unordered_map>

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include "grape/exception.h"

namespace {

//=================================================================================================
template <std::invocable Fn>
class [[nodiscard]] ScopeGuard {
public:
  explicit ScopeGuard(Fn&& fn) : fn_(std::move(fn)) {
  }
  ~ScopeGuard() {
    fn_();
  }

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard(ScopeGuard&&) = delete;
  auto operator=(const ScopeGuard&) = delete;
  auto operator=(ScopeGuard&&) = delete;

private:
  Fn fn_;
};

//-------------------------------------------------------------------------------------------------
[[nodiscard, maybe_unused]] constexpr auto toControlType(unsigned int ev_type)
    -> grape::joystick::ControlType {
  using ControlType = grape::joystick::ControlType;
  switch (ev_type) {
      // clang-format off
    case EV_KEY: return ControlType::Button;
    case EV_ABS: return ControlType::Axis;
    default: return ControlType::Unknown; //TODO: log this
      // clang-format on
  }
}

//-------------------------------------------------------------------------------------------------
[[nodiscard, maybe_unused]] constexpr auto toControlId(unsigned int code)
    -> grape::joystick::ControlId {
  using ControlId = grape::joystick::ControlId;
  switch (code) {
      // clang-format off
    case ABS_X: return ControlId::AxisX ;
    case ABS_Y: return ControlId::AxisY ;
    case ABS_Z: return ControlId::AxisZ ;
    case ABS_RX: return ControlId::AxisRx ;
    case ABS_RY: return ControlId::AxisRy ;
    case ABS_RZ: return ControlId::AxisRz ;
    case ABS_THROTTLE: return ControlId::AxisTrottle ;
    case ABS_RUDDER: return ControlId::AxisRudder ;
    case ABS_WHEEL: return ControlId::AxisWheel ;
    case ABS_GAS: return ControlId::AxisGas ;
    case ABS_BRAKE: return ControlId::AxisBrake ;
    case ABS_HAT0X: return ControlId::AxisHat0X ;
    case ABS_HAT0Y: return ControlId::AxisHat0Y ;
    case ABS_HAT1X: return ControlId::AxisHat1X ;
    case ABS_HAT1Y: return ControlId::AxisHat1Y ;
    case ABS_HAT2X: return ControlId::AxisHat2X ;
    case ABS_HAT2Y: return ControlId::AxisHat2Y ;
    case ABS_HAT3X: return ControlId::AxisHat3X ;
    case ABS_HAT3Y: return ControlId::AxisHat3Y ;
    
    case KEY_UP: return ControlId::KeyUp ;
    case KEY_LEFT: return ControlId::KeyLeft ;
    case KEY_RIGHT: return ControlId::KeyRight ;
    case KEY_DOWN: return ControlId::KeyDown ;
    case KEY_BACK: return ControlId::KeyBack ;
    
    case BTN_0: return ControlId::Button0 ;
    case BTN_1: return ControlId::Button1 ;
    case BTN_2: return ControlId::Button2 ;
    case BTN_3: return ControlId::Button3 ;
    case BTN_4: return ControlId::Button4 ;
    case BTN_5: return ControlId::Button5 ;
    case BTN_6: return ControlId::Button6 ;
    case BTN_7: return ControlId::Button7 ;
    case BTN_8: return ControlId::Button8 ;
    case BTN_9: return ControlId::Button9 ;
    case BTN_DPAD_UP: return ControlId::ButtonDpadUp ;
    case BTN_DPAD_DOWN: return ControlId::ButtonDpadDown ;
    case BTN_DPAD_LEFT: return ControlId::ButtonDpadLeft ;
    case BTN_DPAD_RIGHT: return ControlId::ButtonDpadRight ;
    case BTN_LEFT: return ControlId::ButtonLeft ;
    case BTN_RIGHT    : return ControlId::ButtonRight ;
    case BTN_MIDDLE   : return ControlId::ButtonMiddle ;
    case BTN_SIDE     : return ControlId::ButtonSide ;
    case BTN_EXTRA    : return ControlId::ButtonExtra ;
    case BTN_FORWARD  : return ControlId::ButtonForward ;
    case BTN_BACK     : return ControlId::ButtonBack ;
    case BTN_TASK     : return ControlId::ButtonTask ;
    case BTN_TRIGGER  : return ControlId::ButtonTrigger ;
    case BTN_TRIGGER_HAPPY1: return ControlId::ButtonTriggerHappy1 ;
    case BTN_TRIGGER_HAPPY2: return ControlId::ButtonTriggerHappy2 ;
    case BTN_TRIGGER_HAPPY3: return ControlId::ButtonTriggerHappy3 ;
    case BTN_TRIGGER_HAPPY4: return ControlId::ButtonTriggerHappy4 ;
    case BTN_TRIGGER_HAPPY5: return ControlId::ButtonTriggerHappy5 ;
    case BTN_TRIGGER_HAPPY6: return ControlId::ButtonTriggerHappy6 ;
    case BTN_THUMB      : return ControlId::ButtonThumb ;
    case BTN_THUMB2     : return ControlId::ButtonThumb2 ;
    case BTN_TOP        : return ControlId::ButtonTop ;
    case BTN_TOP2       : return ControlId::ButtonTop2 ;
    case BTN_PINKIE     : return ControlId::ButtonPinkie ;
    case BTN_BASE       : return ControlId::ButtonBase ;
    case BTN_BASE2: return ControlId::ButtonBase2 ;
    case BTN_BASE3: return ControlId::ButtonBase3 ;
    case BTN_BASE4: return ControlId::ButtonBase4 ;
    case BTN_BASE5: return ControlId::ButtonBase5 ;
    case BTN_BASE6: return ControlId::ButtonBase6 ;
    case BTN_DEAD: return ControlId::ButtonDead ;
    case BTN_A: return ControlId::ButtonA ;
    case BTN_B: return ControlId::ButtonB ;
    case BTN_C: return ControlId::ButtonC ;
    case BTN_X: return ControlId::ButtonX ;
    case BTN_Y: return ControlId::ButtonY ;
    case BTN_Z: return ControlId::ButtonZ ;
    case BTN_TL: return ControlId::ButtonTL ;
    case BTN_TR: return ControlId::ButtonTR ;
    case BTN_TL2: return ControlId::ButtonTL2 ;
    case BTN_TR2: return ControlId::ButtonTR2 ;
    case BTN_SELECT: return ControlId::ButtonSelect ;
    case BTN_START: return ControlId::ButtonStart ;
    case BTN_MODE: return ControlId::ButtonMode ;
    case BTN_THUMBL: return ControlId::ButtonThumbL ;
    case BTN_THUMBR: return ControlId::ButtonThumbR ;
    case BTN_WHEEL: return ControlId::ButtonWheel ;
    case BTN_GEAR_UP: return ControlId::ButtonGearUp;
    default: return ControlId::Unknown; //TODO: log this
      // clang-format on
  }
}

//-------------------------------------------------------------------------------------------------
auto readDeviceInfo(const std::filesystem::path& path) -> grape::joystick::DeviceInfo {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto fd = ::open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    const auto err = std::error_code(errno, std::system_category());
    grape::panic<grape::Exception>(
        std::format("Cannot open device '{}': {}", path.string(), err.message()));
  }
  auto guard = ScopeGuard([fd]() { ::close(fd); });

  constexpr auto MAX_NAME_LEN = 256U;
  std::array<char, MAX_NAME_LEN> name = { 0 };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd, EVIOCGNAME(MAX_NAME_LEN), name.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    grape::panic<grape::Exception>(std::format("Cannot get device name: {}", err.message()));
  }

  auto device_info = grape::joystick::DeviceInfo{};
  device_info.name = name.data();
  device_info.path = path;

  auto native_device_id = input_id{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd, EVIOCGID, &native_device_id) != 0) {
    const auto err = std::error_code(errno, std::system_category());
    grape::panic<grape::Exception>(std::format("Cannot get device ID: {}", err.message()));
  }

  device_info.bus_type = native_device_id.bustype;
  device_info.vendor_id = native_device_id.vendor;
  device_info.product_id = native_device_id.product;
  device_info.hw_version = native_device_id.version;

  return device_info;
}

}  // namespace

namespace grape::joystick {

//-------------------------------------------------------------------------------------------------
auto enumerate() -> std::vector<DeviceInfo> {
  static constexpr auto EVDEV_DIR = "/dev/input/by-path/";  // location of evdev devices
  static constexpr auto JSDEV_TOKEN = "event-joystick";     // joystick device identifier

  constexpr auto MAX_EXPECTED_JOYSTICKS = 5U;
  std::vector<DeviceInfo> devices;
  devices.reserve(MAX_EXPECTED_JOYSTICKS);

  for (const auto& entry : std::filesystem::directory_iterator(EVDEV_DIR)) {
    const auto& path = entry.path();
    if (path.string().ends_with(JSDEV_TOKEN)) {
      devices.push_back(readDeviceInfo(path));
    }
  }
  return devices;
}

//=================================================================================================
class Joystick::Impl {
public:
  Impl(std::filesystem::path path, EventCallback&& cb);
  auto open() -> bool;
  void close();

private:
  auto pollForEvents() noexcept -> bool;
  auto waitForEvents() noexcept -> bool;
  void eventLoop(const std::stop_token& st) noexcept;

  std::filesystem::path device_path_;
  std::atomic_int fd_{ -1 };
  std::atomic_bool attempt_reconnection_{ true };
  EventCallback event_cb_{ nullptr };
  std::jthread event_thread_;
};

//-------------------------------------------------------------------------------------------------
Joystick::Impl::Impl(std::filesystem::path path, EventCallback&& cb)
  : device_path_(std::move(path)), event_cb_(std::move(cb)) {
  event_thread_ = std::jthread([this](const std::stop_token& st) { eventLoop(st); });
}

//-------------------------------------------------------------------------------------------------
void Joystick::Impl::close() {
  if (fd_ < 0) {
    return;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  std::ignore = ::ioctl(fd_, EVIOCGRAB, 0);
  ::close(fd_);
  fd_ = -1;
  if (event_cb_) {
    event_cb_(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = false });
  }
}

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::open() -> bool {
  close();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  fd_ = ::open(device_path_.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd_ < 0) {
    const auto err_num = errno;
    if (err_num == ENOENT) {
      return false;
    }
    const auto err = std::error_code(err_num, std::system_category());
    grape::panic<grape::Exception>(std::format("Cannot open device: {}", err.message()));
  }

  // Get exclusive access to device to avoid missing and duplicate events
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd_, EVIOCGRAB, 1) != 0) {
    const auto err = std::error_code(errno, std::system_category());
    grape::panic<grape::Exception>(std::format("Cannot get exclusive access: {}", err.message()));
  }

  if (event_cb_ != nullptr) {
    event_cb_(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = true });
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::pollForEvents() noexcept -> bool {
  // const auto ts = Clock::now();
  //  read axis, keys, buttons
  //  call event_cb for each
  //  set attempt_reconnection in case of errors and log the error (dont throw)
  return true;
}

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::waitForEvents() noexcept -> bool {
  // set attempt_reconnection in case of errors and log the error (dont throw)
  return true;
}

//-------------------------------------------------------------------------------------------------
void Joystick::Impl::eventLoop(const std::stop_token& st) noexcept {
  try {
    constexpr auto RECONNECTION_DELAY = std::chrono::seconds(1);
    while (not st.stop_requested()) {
      if (attempt_reconnection_) {
        std::this_thread::sleep_for(RECONNECTION_DELAY);
        if (open()) {
          attempt_reconnection_ = (not pollForEvents());
        }
      } else {
        attempt_reconnection_ = (not waitForEvents());
      }
    }
    close();
  } catch (...) {
    if (event_cb_ != nullptr) {
      event_cb_(ErrorEvent{ .timestamp = Clock::now(), .exception = std::current_exception() });
      event_cb_(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = false });
    }
  }
}

//-------------------------------------------------------------------------------------------------
Joystick::Joystick(const std::filesystem::path& device_path, EventCallback&& cb)
  : impl_(std::make_unique<Impl>(device_path, std::move(cb))) {
}

//-------------------------------------------------------------------------------------------------
Joystick::~Joystick() = default;

}  // namespace grape::joystick
