//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/joystick/joystick.h"

#include <concepts>  //std::invocable
#include <cstring>
#include <expected>
#include <format>
#include <thread>

#include <fcntl.h>
#include <linux/input.h>
#include <sys/epoll.h>
#include <unistd.h>

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
constexpr auto toControlType(unsigned int ev_type) -> grape::joystick::ControlType {
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
constexpr auto toControlId(unsigned int code) -> grape::joystick::ControlId {
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
auto toTimePoint(const timeval& tv) -> grape::joystick::Clock::time_point {
  return grape::joystick::Clock::time_point{ std::chrono::seconds(tv.tv_sec) +
                                             std::chrono::microseconds(tv.tv_usec) };
}

//-------------------------------------------------------------------------------------------------
auto toControlEvent(input_event ev) -> grape::joystick::ControlEvent {
  return { .timestamp = toTimePoint(ev.time),
           .type = toControlType(ev.type),
           .id = toControlId(ev.code),
           .value = ev.value };
}

}  // namespace

namespace grape::joystick {

//-------------------------------------------------------------------------------------------------
auto enumerate() -> std::vector<std::filesystem::path> {
  static constexpr auto EVDEV_DIR = "/dev/input/by-path/";  // location of evdev devices
  static constexpr auto JSDEV_TOKEN = "event-joystick";     // joystick device identifier

  constexpr auto MAX_EXPECTED_JOYSTICKS = 5U;
  auto devices = std::vector<std::filesystem::path>();
  devices.reserve(MAX_EXPECTED_JOYSTICKS);

  for (const auto& entry : std::filesystem::directory_iterator(EVDEV_DIR)) {
    const auto& path = entry.path();
    if (path.string().ends_with(JSDEV_TOKEN)) {
      devices.push_back(path);
    }
  }
  return devices;
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
  auto guard = ScopeGuard([fd]() -> void { ::close(fd); });

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

//=================================================================================================
struct Joystick::Impl {
  int device_fd{ -1 };
  int epoll_fd{ -1 };
  bool is_initialised{ false };  // TODO(vilas): rename to poll_state?
  Capabilities capabilities;
  EventCallback callback{ nullptr };
};

//-------------------------------------------------------------------------------------------------
auto Joystick::readCapabilities() -> std::expected<Capabilities, std::string> {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

  auto capabilities = Capabilities{};

  // read all control types on device
  auto type_codes_bitmap = BitSet<EV_MAX>{};
  if (::ioctl(impl_->device_fd, EVIOCGBIT(0, EV_MAX), type_codes_bitmap.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("EVIOCGBIT(type): {}", err.message()) };
  }

  // For all control types we support..
  for (const auto ev_type : std::array<unsigned int, 2>{ EV_KEY, EV_ABS }) {
    // ..test if type is supported on the device
    if (type_codes_bitmap.check(ev_type)) {
      // read all controls of the type
      auto key_codes_bitmap = BitSet<KEY_MAX>{};
      if (::ioctl(impl_->device_fd, EVIOCGBIT(ev_type, KEY_MAX), key_codes_bitmap.data()) < 0) {
        const auto err = std::error_code(errno, std::system_category());
        return std::unexpected{ std::format("EVIOCGBIT(controls): {}", err.message()) };
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
Joystick::Joystick(EventCallback&& cb) : impl_(std::make_unique<Impl>()) {
  impl_->callback = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
Joystick::~Joystick() {
  close();
}

//-------------------------------------------------------------------------------------------------
auto Joystick::open(const std::filesystem::path& device_path) -> bool {
  close();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  impl_->device_fd = ::open(device_path.c_str(), O_RDONLY | O_NONBLOCK);
  if (impl_->device_fd < 0) {
    const auto err = std::error_code(errno, std::system_category());
    impl_->callback(
        ErrorEvent{ .timestamp = Clock::now(),
                    .message = std::format("{}: {}", device_path.c_str(), err.message()) });
    return false;
  }

  // Get exclusive access to device to avoid missing and duplicate events
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(impl_->device_fd, EVIOCGRAB, 1) != 0) {
    const auto err = std::error_code(errno, std::system_category());
    close();
    impl_->callback(
        ErrorEvent{ .timestamp = Clock::now(),
                    .message = std::format("Cannot get exclusive access: {}", err.message()) });
    return false;
  }

  impl_->epoll_fd = ::epoll_create1(0);
  if (impl_->epoll_fd < 0) {  // TODO(vilas): handle error
    ::close(impl_->device_fd);
    impl_->device_fd = -1;
    return false;
  }

  struct epoll_event ev{};
  ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
  ev.data.fd = impl_->device_fd;
  if (::epoll_ctl(impl_->epoll_fd, EPOLL_CTL_ADD, impl_->device_fd, &ev) == -1) {
    ::close(impl_->device_fd);  // TODO(vilas): handle error
    ::close(impl_->epoll_fd);
    impl_->device_fd = -1;
    impl_->epoll_fd = -1;
    return false;
  }

  const auto caps_result = readCapabilities();
  if (not caps_result) {
    close();
    impl_->callback(
        ErrorEvent{ .timestamp = Clock::now(),
                    .message = std::format("Cannot read capabilities: {}", caps_result.error()) });
    return false;
  }
  impl_->capabilities = caps_result.value();
  impl_->is_initialised = false;
  impl_->callback(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = true });
  return true;
}

//-------------------------------------------------------------------------------------------------
void Joystick::close() {
  if (impl_->device_fd >= 0) {
    std::ignore = ::ioctl(impl_->device_fd, EVIOCGRAB, 0);
    ::close(impl_->device_fd);
    impl_->device_fd = -1;
  }
  if (impl_->epoll_fd >= 0) {
    ::close(impl_->epoll_fd);
    impl_->epoll_fd = -1;
  }
  impl_->is_initialised = false;
  impl_->callback(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = false });
}

//-------------------------------------------------------------------------------------------------
auto Joystick::wait() -> bool {
  return wait(std::chrono::milliseconds(-1));
}

//-------------------------------------------------------------------------------------------------
auto Joystick::wait(std::chrono::milliseconds timeout) -> bool {
  if (impl_->device_fd < 0 || impl_->epoll_fd < 0) {
    return false;
  }

  // On first call, synthesise initial state events
  if (not impl_->is_initialised) {
    readState();  // TODO(vilas): handle error
    impl_->is_initialised = true;
  }

  struct epoll_event ev{};
  const auto nfds = epoll_wait(impl_->epoll_fd, &ev, 1, static_cast<int>(timeout.count()));
  if (nfds == 0) {
    return true;
  }
  if (nfds < 0) {
    return false;  // TODO(vilas): handle error
  }
  if (ev.data.fd == impl_->device_fd) {
    if (0u != (ev.events & (EPOLLHUP | EPOLLERR))) {
      close();
      return false;
    }
    struct input_event iev{};
    ssize_t rd = 0;
    while ((rd = ::read(impl_->device_fd, &iev, sizeof(iev))) == sizeof(iev)) {
      if ((iev.type == EV_SYN) && (iev.code == SYN_DROPPED)) {
        impl_->callback(
            ErrorEvent{ .timestamp = Clock::now(), .message = "One or more events dropped" });
      }

      if ((iev.type == EV_ABS) || (iev.type == EV_KEY)) {
        impl_->callback(toControlEvent(iev));
      }
    }
    if (rd == 0) {
      close();
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
auto Joystick::readState() -> bool {  // TODO(vilas): Read buttons
  const auto tp = Clock::now();

  // Report button state
  unsigned char keys[KEY_MAX / 8 + 1];
  std::memset(keys, 0, sizeof(keys));
  if (::ioctl(impl_->device_fd, EVIOCGKEY(sizeof(keys)), keys) >= 0) {
    for (auto code = 0U; code < KEY_MAX; ++code) {
      if (::ioctl(impl_->device_fd, EVIOCGBIT(EV_KEY, KEY_MAX), nullptr) >= 0) {  // TODO(vilas):
                                                                                  // handle error
        const auto byte = code / 8U;
        const auto bit = code % 8U;
        bool pressed = (keys[byte] & (1U << bit));
        // Only report if code is actually supported as a button
        // Optionally, filter for joystick/gamepad button ranges
        impl_->callback(ControlEvent{ .timestamp = tp,
                                      .type = ControlType::Button,
                                      .id = toControlId(code),
                                      .value = pressed ? 1 : 0 });
      }
    }
  }

  // Report axis state
  unsigned long absbits[(ABS_MAX / (8U * sizeof(unsigned long))) + 1]{};
  if (ioctl(impl_->device_fd, EVIOCGBIT(EV_ABS, sizeof(absbits)), absbits) >= 0) {
    for (auto code = 0U; code < ABS_MAX; ++code) {
      if (absbits[code / (8U * sizeof(unsigned long))] &
          (1UL << (code % (8U * sizeof(unsigned long))))) {
        struct input_absinfo absinfo{};
        if (::ioctl(impl_->device_fd, EVIOCGABS(code), &absinfo) >= 0) {  // TODO(vilas): handle
                                                                          // error
          impl_->callback(ControlEvent{ .timestamp = tp,
                                        .type = ControlType::Axis,
                                        .id = toControlId(code),
                                        .value = absinfo.value });
        }
      }
    }
  }
  return true;  // TODO(vilas): handle error
}
}  // namespace grape::joystick
