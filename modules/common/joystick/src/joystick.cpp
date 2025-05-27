//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/joystick/joystick.h"

#include <atomic>
#include <concepts>  //std::invocable
#include <cstring>
#include <expected>
#include <format>
#include <thread>
#include <unordered_map>

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include "grape/exception.h"

namespace {

//=================================================================================================
template <std::invocable ExitFn>
class [[nodiscard]] ScopeGuard {
public:
  explicit ScopeGuard(ExitFn&& fn) : exit_fn_(std::move(fn)) {
  }
  ~ScopeGuard() {
    exit_fn_();
  }

  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard(ScopeGuard&&) = delete;
  auto operator=(const ScopeGuard&) = delete;
  auto operator=(ScopeGuard&&) = delete;

private:
  ExitFn exit_fn_;
};

//=================================================================================================
/// Mimics std::bitset but provides a subset of functionality
template <std::size_t N>
class BitSet {
public:
  /// @return true if bit at index is set
  [[nodiscard]] constexpr auto check(const std::size_t& index) const -> bool {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    const auto byte_idx = index / 8U;
    const auto bit_idx = index % 8U;
    return ((data_.at(byte_idx) & (1U << bit_idx)) != 0);
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
  }

  /// @return Direct access to internal buffer
  [[nodiscard]] constexpr auto data() -> std::uint8_t* {
    return data_.data();
  }

private:
  static constexpr auto NUM_BYTES = (N + 7U) / 8U;  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  static constexpr auto STORAGE_SIZE = (NUM_BYTES > 0U ? NUM_BYTES : 1U);
  std::array<std::uint8_t, STORAGE_SIZE> data_{};
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toControlType(unsigned int ev_type) -> grape::joystick::ControlType {
  using ControlType = grape::joystick::ControlType;
  switch (ev_type) {
      // clang-format off
    case EV_KEY: return ControlType::Button;
    case EV_ABS: return ControlType::Axis;
    default: return ControlType::Unknown;
      // clang-format on
  }
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toControlId(unsigned int code) -> grape::joystick::ControlId {
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
    default: return ControlId::Unknown;
      // clang-format on
  }
}

//-------------------------------------------------------------------------------------------------
auto readDeviceInfo(const std::filesystem::path& path)
    -> std::expected<grape::joystick::DeviceInfo, std::string> {
  const auto* const path_str = path.c_str();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto fd = ::open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("Cannot open device '{}': {}", path_str, err.message()) };
  }
  auto guard = ScopeGuard([fd]() { ::close(fd); });

  constexpr auto MAX_NAME_LEN = 256U;
  std::array<char, MAX_NAME_LEN> name = { 0 };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd, EVIOCGNAME(MAX_NAME_LEN), name.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("Cannot get device name '{}': {}", path_str,
                                        err.message()) };
  }

  auto device_info = grape::joystick::DeviceInfo{};
  device_info.name = name.data();
  device_info.path = path;

  auto native_device_id = input_id{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd, EVIOCGID, &native_device_id) != 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("Cannot get device ID '{}': {}", path_str, err.message()) };
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
      if (const auto& info = readDeviceInfo(path); info.has_value()) {
        devices.push_back(info.value());
      }
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
  auto readCapabilities() -> std::expected<Capabilities, std::string>;
  auto pollForEvents() -> bool;
  auto waitForEvents() -> bool;
  void eventLoop(const std::stop_token& st);

  std::filesystem::path device_path_;
  std::atomic_int fd_{ -1 };
  Capabilities capabilities_;
  EventCallback event_cb_{ nullptr };
  std::jthread event_thread_;
};

//-------------------------------------------------------------------------------------------------
Joystick::Impl::Impl(std::filesystem::path path, EventCallback&& cb)
  : device_path_(std::move(path)), event_cb_(std::move(cb)) {
  if (event_cb_ == nullptr) {
    throw std::runtime_error("Event callback is invalid");
  }
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
  capabilities_.clear();
  event_cb_(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = false });
}

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::open() -> bool {
  close();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  fd_ = ::open(device_path_.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd_ < 0) {
    const auto err = std::error_code(errno, std::system_category());
    event_cb_(ErrorEvent{ .timestamp = Clock::now(),
                          .message = std::format("{}: {}", device_path_.c_str(), err.message()) });
    return false;
  }

  // Get exclusive access to device to avoid missing and duplicate events
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd_, EVIOCGRAB, 1) != 0) {
    const auto err = std::error_code(errno, std::system_category());
    close();
    event_cb_(
        ErrorEvent{ .timestamp = Clock::now(),
                    .message = std::format("Cannot get exclusive access: {}", err.message()) });
    return false;
  }

  const auto caps_result = readCapabilities();
  if (not caps_result) {
    close();
    event_cb_(
        ErrorEvent{ .timestamp = Clock::now(),
                    .message = std::format("Cannot read capabilities: {}", caps_result.error()) });
    return false;
  }
  capabilities_ = caps_result.value();

  event_cb_(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = true });
  return {};
}

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::readCapabilities() -> std::expected<Capabilities, std::string> {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

  auto capabilities = Capabilities{};

  // read all control types on device
  auto type_codes_bitmap = BitSet<EV_MAX>{};
  if (::ioctl(fd_, EVIOCGBIT(0, EV_MAX), type_codes_bitmap.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("EVIOCGBIT(0): {}", err.message()) };
  }

  // For all control types we support..
  for (const auto ev_type : std::array<unsigned int, 2>{ EV_KEY, EV_ABS }) {
    // ..test if type is supported on the device
    if (type_codes_bitmap.check(ev_type)) {
      // read all controls of the type
      auto key_codes_bitmap = BitSet<KEY_MAX>{};
      if (::ioctl(fd_, EVIOCGBIT(ev_type, KEY_MAX), key_codes_bitmap.data()) < 0) {
        const auto err = std::error_code(errno, std::system_category());
        return std::unexpected{ std::format("EVIOCGBIT(type): {}", err.message()) };
      }

      // iterate through all controls of a type
      std::vector<ControlId> available_controls;
      available_controls.reserve(KEY_MAX);
      for (unsigned int code = 0; code < KEY_MAX; code++) {
        // test if control is available
        if (key_codes_bitmap.check(code)) {
          const auto id = toControlId(code);
          if (id == ControlId::Unknown) {
            std::ignore = std::fprintf(stderr, "Ignored unsupported event code %x", code);
            continue;
          }
          available_controls.push_back(id);
        }  // if control is available
      }  // for each control of a type
      capabilities[toControlType(ev_type)] = available_controls;
    }  // if type is available
  }  // for supported types

  // NOLINTEND(cppcoreguidelines-pro-type-vararg)

  return capabilities;
}

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::pollForEvents() -> bool {
  // const auto ts = Clock::now();
  //  read axis, keys, buttons
  //  call event_cb for each
  //  set attempt_reconnection in case of errors and log the error (dont throw)
  return {};
}

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::waitForEvents() -> bool {
  // set attempt_reconnection in case of errors and log the error (dont throw)
  return {};
}

//-------------------------------------------------------------------------------------------------
void Joystick::Impl::eventLoop(const std::stop_token& st) {
  try {
    constexpr auto RECONNECTION_DELAY = std::chrono::seconds(1);
    auto is_connected = false;
    while (not st.stop_requested()) {
      if (not is_connected) {
        std::this_thread::sleep_for(RECONNECTION_DELAY);
        if (not open()) {
          continue;
        }
        is_connected = pollForEvents();
      } else {
        is_connected = waitForEvents();
      }
    }
    close();
  } catch (const std::exception& ex) {
    event_cb_(ErrorEvent{ .timestamp = Clock::now(), .message = ex.what() });
    event_cb_(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = false });
  }
}

//-------------------------------------------------------------------------------------------------
Joystick::Joystick(const std::filesystem::path& device_path, EventCallback&& cb)
  : impl_(std::make_unique<Impl>(device_path, std::move(cb))) {
}

//-------------------------------------------------------------------------------------------------
Joystick::~Joystick() = default;

}  // namespace grape::joystick
