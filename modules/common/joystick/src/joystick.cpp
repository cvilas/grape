//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/joystick/joystick.h"

#include <climits>
#include <concepts>  //std::invocable
#include <cstring>
#include <expected>
#include <format>
#include <unordered_map>

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
    const auto byte_idx = index / CHAR_BIT;
    const auto bit_idx = index % CHAR_BIT;
    return ((data_.at(byte_idx) & (1U << bit_idx)) != 0);
  }

  /// @return Direct access to internal buffer
  [[nodiscard]] constexpr auto data() -> std::uint8_t* {
    return data_.data();
  }

  /// @return size of internal buffer
  [[nodiscard]] constexpr auto length() const -> std::size_t {
    return data_.size();
  }

private:
  static constexpr auto STORAGE_BYTES = (N / CHAR_BIT) + 1U;
  std::array<std::uint8_t, STORAGE_BYTES> data_{};
};

//-------------------------------------------------------------------------------------------------
[[maybe_unused]] constexpr auto toControlType(unsigned int ev_type)
    -> grape::joystick::ControlType {
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
[[nodiscard]] constexpr auto toControlId(unsigned int ev_code) -> grape::joystick::ControlId {
  using ControlId = grape::joystick::ControlId;
  switch (ev_code) {
      // clang-format off
    case ABS_X              : return ControlId::AxisX ;
    case ABS_Y              : return ControlId::AxisY ;
    case ABS_Z              : return ControlId::AxisZ ;
    case ABS_RX             : return ControlId::AxisRx ;
    case ABS_RY             : return ControlId::AxisRy ;
    case ABS_RZ             : return ControlId::AxisRz ;
    case ABS_THROTTLE       : return ControlId::AxisThrottle ;
    case ABS_RUDDER         : return ControlId::AxisRudder ;
    case ABS_WHEEL          : return ControlId::AxisWheel ;
    case ABS_GAS            : return ControlId::AxisGas ;
    case ABS_BRAKE          : return ControlId::AxisBrake ;
    case ABS_HAT0X          : return ControlId::AxisHat0X ;
    case ABS_HAT0Y          : return ControlId::AxisHat0Y ;
    case ABS_HAT1X          : return ControlId::AxisHat1X ;
    case ABS_HAT1Y          : return ControlId::AxisHat1Y ;
    case ABS_HAT2X          : return ControlId::AxisHat2X ;
    case ABS_HAT2Y          : return ControlId::AxisHat2Y ;
    case ABS_HAT3X          : return ControlId::AxisHat3X ;
    case ABS_HAT3Y          : return ControlId::AxisHat3Y ;
          
    case KEY_UP             : return ControlId::KeyUp ;
    case KEY_LEFT           : return ControlId::KeyLeft ;
    case KEY_RIGHT          : return ControlId::KeyRight ;
    case KEY_DOWN           : return ControlId::KeyDown ;
    case KEY_BACK           : return ControlId::KeyBack ;
          
    case BTN_0              : return ControlId::Button0 ;
    case BTN_1              : return ControlId::Button1 ;
    case BTN_2              : return ControlId::Button2 ;
    case BTN_3              : return ControlId::Button3 ;
    case BTN_4              : return ControlId::Button4 ;
    case BTN_5              : return ControlId::Button5 ;
    case BTN_6              : return ControlId::Button6 ;
    case BTN_7              : return ControlId::Button7 ;
    case BTN_8              : return ControlId::Button8 ;
    case BTN_9              : return ControlId::Button9 ;
    case BTN_DPAD_UP        : return ControlId::ButtonDpadUp ;
    case BTN_DPAD_DOWN      : return ControlId::ButtonDpadDown ;
    case BTN_DPAD_LEFT      : return ControlId::ButtonDpadLeft ;
    case BTN_DPAD_RIGHT     : return ControlId::ButtonDpadRight ;
    case BTN_LEFT           : return ControlId::ButtonLeft ;
    case BTN_RIGHT          : return ControlId::ButtonRight ;
    case BTN_MIDDLE         : return ControlId::ButtonMiddle ;
    case BTN_SIDE           : return ControlId::ButtonSide ;
    case BTN_EXTRA          : return ControlId::ButtonExtra ;
    case BTN_FORWARD        : return ControlId::ButtonForward ;
    case BTN_BACK           : return ControlId::ButtonBack ;
    case BTN_TASK           : return ControlId::ButtonTask ;
    case BTN_TRIGGER        : return ControlId::ButtonTrigger ;
    case BTN_TRIGGER_HAPPY1 : return ControlId::ButtonTriggerHappy1 ;
    case BTN_TRIGGER_HAPPY2 : return ControlId::ButtonTriggerHappy2 ;
    case BTN_TRIGGER_HAPPY3 : return ControlId::ButtonTriggerHappy3 ;
    case BTN_TRIGGER_HAPPY4 : return ControlId::ButtonTriggerHappy4 ;
    case BTN_TRIGGER_HAPPY5 : return ControlId::ButtonTriggerHappy5 ;
    case BTN_TRIGGER_HAPPY6 : return ControlId::ButtonTriggerHappy6 ;
    case BTN_THUMB          : return ControlId::ButtonThumb ;
    case BTN_THUMB2         : return ControlId::ButtonThumb2 ;
    case BTN_TOP            : return ControlId::ButtonTop ;
    case BTN_TOP2           : return ControlId::ButtonTop2 ;
    case BTN_PINKIE         : return ControlId::ButtonPinkie ;
    case BTN_BASE           : return ControlId::ButtonBase ;
    case BTN_BASE2          : return ControlId::ButtonBase2 ;
    case BTN_BASE3          : return ControlId::ButtonBase3 ;
    case BTN_BASE4          : return ControlId::ButtonBase4 ;
    case BTN_BASE5          : return ControlId::ButtonBase5 ;
    case BTN_BASE6          : return ControlId::ButtonBase6 ;
    case (BTN_BASE6+1)      : return ControlId::ButtonBase7 ; //unofficial extensions
    case (BTN_BASE6+2)      : return ControlId::ButtonBase8 ; // ..
    case (BTN_BASE6+3)      : return ControlId::ButtonBase9 ; // ..
    case BTN_DEAD           : return ControlId::ButtonDead ;
    case BTN_A              : return ControlId::ButtonA ;
    case BTN_B              : return ControlId::ButtonB ;
    case BTN_C              : return ControlId::ButtonC ;
    case BTN_X              : return ControlId::ButtonX ;
    case BTN_Y              : return ControlId::ButtonY ;
    case BTN_Z              : return ControlId::ButtonZ ;
    case BTN_TL             : return ControlId::ButtonTL ;
    case BTN_TR             : return ControlId::ButtonTR ;
    case BTN_TL2            : return ControlId::ButtonTL2 ;
    case BTN_TR2            : return ControlId::ButtonTR2 ;
    case BTN_SELECT         : return ControlId::ButtonSelect ;
    case BTN_START          : return ControlId::ButtonStart ;
    case BTN_MODE           : return ControlId::ButtonMode ;
    case BTN_THUMBL         : return ControlId::ButtonThumbL ;
    case BTN_THUMBR         : return ControlId::ButtonThumbR ;
    case BTN_WHEEL          : return ControlId::ButtonWheel ;
    case BTN_GEAR_UP        : return ControlId::ButtonGearUp;
    default: return ControlId::Unknown;
      // clang-format on
  }
}

//-------------------------------------------------------------------------------------------------
auto toTimePoint(const timeval& tv) -> grape::joystick::Clock::time_point {
  return grape::joystick::Clock::time_point{ std::chrono::seconds(tv.tv_sec) +
                                             std::chrono::microseconds(tv.tv_usec) };
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
auto readDeviceInfo(const std::filesystem::path& path) -> std::expected<DeviceInfo, std::string> {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  const auto fd = ::open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("Cannot open device: {}", err.message()) };
  }

  auto guard = ScopeGuard([fd]() -> void { ::close(fd); });

  constexpr auto MAX_NAME_LEN = 256U;
  std::array<char, MAX_NAME_LEN> name = { 0 };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd, EVIOCGNAME(MAX_NAME_LEN), name.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("Cannot get device name: {}", err.message()) };
  }

  auto native_device_id = input_id{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(fd, EVIOCGID, &native_device_id) != 0) {
    const auto err = std::error_code(errno, std::system_category());
    return std::unexpected{ std::format("Cannot get device ID: {}", err.message()) };
  }

  return DeviceInfo{ .name = name.data(),
                     .path = path,
                     .bus_type = native_device_id.bustype,
                     .vendor_id = native_device_id.vendor,
                     .product_id = native_device_id.product,
                     .hw_version = native_device_id.version };
}

//=================================================================================================
struct Joystick::Impl {
  [[nodiscard]] auto normalise(ControlId axis, std::int32_t value) const -> float;
  struct AxisRange {
    float center{};
    float scale{};
  };
  static constexpr auto INVALID_FD = -1;
  int device_fd{ INVALID_FD };
  int epoll_fd{ INVALID_FD };
  std::unordered_map<ControlId, AxisRange> range;
  EventCallback callback{ nullptr };
};

//-------------------------------------------------------------------------------------------------
auto Joystick::Impl::normalise(ControlId axis, std::int32_t value) const -> float {
  auto it = range.find(axis);
  if (it == range.end()) {
    return 0.F;
  }
  const auto& axis_range = it->second;
  return (static_cast<float>(value) - axis_range.center) * axis_range.scale;
}

//-------------------------------------------------------------------------------------------------
Joystick::Joystick(EventCallback&& cb) : impl_(std::make_unique<Impl>()) {
  impl_->callback = std::move(cb);
}

//-------------------------------------------------------------------------------------------------
Joystick::~Joystick() {
  try {
    close();
  } catch (const std::exception& ex) {
    (void)std::fputs(ex.what(), stderr);
  }
}

//-------------------------------------------------------------------------------------------------
auto Joystick::open(const std::filesystem::path& device_path) const -> bool {
  close();

  const auto tp = Clock::now();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  impl_->device_fd = ::open(device_path.c_str(), O_RDONLY | O_NONBLOCK);
  if (impl_->device_fd < 0) {
    const auto err = std::error_code(errno, std::system_category());
    impl_->callback(ErrorEvent{ .timestamp = Clock::now(),
                                .message = std::format("Cannot open device: {}", err.message()) });
    return false;
  }

  // Get exclusive access to device avoids missing or duplicate events
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(impl_->device_fd, EVIOCGRAB, 1) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    close();
    impl_->callback(
        ErrorEvent{ .timestamp = tp,
                    .message = std::format("Cannot get exclusive access: {}", err.message()) });
    return false;
  }

  impl_->epoll_fd = ::epoll_create1(0);
  if (impl_->epoll_fd < 0) {
    const auto err = std::error_code(errno, std::system_category());
    close();
    impl_->callback(ErrorEvent{
        .timestamp = tp, .message = std::format("Cannot initialise epoll: {}", err.message()) });
    return false;
  }

  auto ev = epoll_event{};
  ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
  ev.data.fd = impl_->device_fd;
  if (::epoll_ctl(impl_->epoll_fd, EPOLL_CTL_ADD, impl_->device_fd, &ev) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    close();
    impl_->callback(ErrorEvent{
        .timestamp = tp, .message = std::format("Cannot configure epoll: {}", err.message()) });
    return false;
  }
  impl_->callback(ConnectionEvent{ .timestamp = tp, .is_connected = true });

  return readState();
}

//-------------------------------------------------------------------------------------------------
void Joystick::close() const {
  const auto was_open = (impl_->epoll_fd >= 0);
  if (was_open) {
    ::close(impl_->epoll_fd);
    impl_->epoll_fd = Impl::INVALID_FD;
  }
  if (impl_->device_fd >= 0) {
    std::ignore = ::ioctl(impl_->device_fd, EVIOCGRAB, 0);
    ::close(impl_->device_fd);
    impl_->device_fd = Impl::INVALID_FD;
  }
  impl_->range.clear();
  if (was_open) {  // raise disconnect event once per connect event
    impl_->callback(ConnectionEvent{ .timestamp = Clock::now(), .is_connected = false });
  }
}

//-------------------------------------------------------------------------------------------------
auto Joystick::process(std::chrono::milliseconds timeout) const -> bool {
  if (impl_->device_fd < 0 || impl_->epoll_fd < 0) {
    impl_->callback(ErrorEvent{ .timestamp = Clock::now(), .message = "Device not open" });
    return false;
  }

  auto ev = epoll_event{};
  const auto nfds = epoll_wait(impl_->epoll_fd, &ev, 1, static_cast<int>(timeout.count()));
  if (nfds < 0) {
    const auto err = std::error_code(errno, std::system_category());
    impl_->callback(ErrorEvent{ .timestamp = Clock::now(),
                                .message = std::format("epoll_wait: {}", err.message()) });
    return false;
  }

  if (nfds == 0) {
    return true;  // no pending events
  }

  if (0U != (ev.events & (EPOLLHUP | EPOLLERR))) {
    impl_->callback(ErrorEvent{ .timestamp = Clock::now(), .message = "Device error or hang-up" });
    return false;
  }

  static constexpr auto MAX_EVENTS_PER_CALL = 64U;
  for (auto event_count = 0U; event_count < MAX_EVENTS_PER_CALL; ++event_count) {
    auto iev = input_event{};
    static constexpr auto EVENT_SZ = sizeof(input_event);
    const auto bytes_read = ::read(impl_->device_fd, &iev, EVENT_SZ);
    if (bytes_read < 0) {
      const auto err = std::error_code(errno, std::system_category());
      if (errno == EAGAIN) {
        return true;  // No more events to read
      }
      impl_->callback(ErrorEvent{ .timestamp = Clock::now(),
                                  .message = std::format("Read failed: {}", err.message()) });
      return false;
    }
    if (std::cmp_less(bytes_read, EVENT_SZ)) {
      impl_->callback(ErrorEvent{ .timestamp = Clock::now(), .message = "Partial read" });
      return false;
    }
    if ((iev.type == EV_SYN) && (iev.code == SYN_DROPPED)) {
      // @note: If the client is not fast enough in clearing the events, the kernel places
      // EV_SYN:SYN_DROPPED into the buffer, effectively notifying us that some events were lost.
      // A SYN_DROPPED event may be recieved at any time in the event sequence. According to the
      // docs, when a SYN_DROPPED event is received, we must:
      // 1. Discard all events since the last SYN_REPORT
      // 2. Discard all events until including the next SYN_REPORT These event are part of
      //   incomplete event frames.
      // In our case, since we process events as we go, we may have already reported incomplete
      // event frames by the get we get here. Not much to do other than to report this as error
      // and let user reinitialise the device. The only solution is for the user application
      // to ensure that it processes events fast enough.
      impl_->callback(ErrorEvent{ .timestamp = Clock::now(), .message = "Events dropped" });
      return false;
    }
    if (iev.type == EV_ABS) {
      const auto control_id = toControlId(iev.code);
      impl_->callback(AxisEvent{ .timestamp = toTimePoint(iev.time),
                                 .id = control_id,
                                 .value = impl_->normalise(control_id, iev.value) });
    }
    if (iev.type == EV_KEY) {
      impl_->callback(ButtonEvent{ .timestamp = toTimePoint(iev.time),
                                   .id = toControlId(iev.code),
                                   .pressed = (iev.value != 0) });
    }
  }

  return true;
}

//-------------------------------------------------------------------------------------------------
auto Joystick::readState() const -> bool {
  const auto tp = Clock::now();

  // Get available buttons on the device
  auto keys_map = BitSet<KEY_MAX>{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(impl_->device_fd, EVIOCGBIT(EV_KEY, KEY_MAX), keys_map.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    impl_->callback(ErrorEvent{
        .timestamp = tp, .message = std::format("Unable to read button map: {}", err.message()) });
    return false;
  }

  // read all button states at once
  auto keys_state = BitSet<KEY_MAX>{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(impl_->device_fd, EVIOCGKEY(keys_state.length()), keys_state.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    impl_->callback(ErrorEvent{
        .timestamp = tp, .message = std::format("Unable to read buttons: {}", err.message()) });
    return false;
  }

  // Report button state
  for (auto code = 0U; code < KEY_MAX; ++code) {
    if (not keys_map.check(code)) {
      continue;
    }
    const auto control_id = toControlId(code);
    const auto pressed = keys_state.check(code);
    impl_->callback(ButtonEvent{ .timestamp = tp, .id = control_id, .pressed = pressed });
  }

  // Get available axes on the device
  auto axes_map = BitSet<ABS_MAX>{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  if (::ioctl(impl_->device_fd, EVIOCGBIT(EV_ABS, ABS_MAX), axes_map.data()) < 0) {
    const auto err = std::error_code(errno, std::system_category());
    impl_->callback(ErrorEvent{
        .timestamp = tp, .message = std::format("Unable to read axes map: {}", err.message()) });
    return false;
  }

  // Read axes state
  for (auto code = 0U; code < ABS_MAX; ++code) {
    if (not axes_map.check(code)) {
      continue;
    }
    const auto control_id = toControlId(code);
    auto absinfo = input_absinfo{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (::ioctl(impl_->device_fd, EVIOCGABS(code), &absinfo) < 0) {
      const auto err = std::error_code(errno, std::system_category());
      impl_->callback(ErrorEvent{ .timestamp = tp,
                                  .message = std::format("Unable to read axis '{}': {}",
                                                         toString(control_id), err.message()) });
      return false;
    }
    const auto range = absinfo.maximum - absinfo.minimum;
    impl_->range[control_id] = {
      // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
      .center = static_cast<float>(absinfo.maximum + absinfo.minimum) / 2.F,
      .scale = (range != 0) ? (2.F / static_cast<float>(range)) : 0.F
      // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
    };
    impl_->callback(AxisEvent{
        .timestamp = tp, .id = control_id, .value = impl_->normalise(control_id, absinfo.value) });
  }
  return true;
}

}  // namespace grape::joystick
