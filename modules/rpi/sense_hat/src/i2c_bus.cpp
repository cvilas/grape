//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/rpi/sense_hat/i2c_bus.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <iterator>
#include <limits>
#include <system_error>
#include <utility>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace grape::rpi::sense_hat {

//-------------------------------------------------------------------------------------------------
auto I2CBus::open(const std::filesystem::path& bus_path) -> std::expected<I2CBus, Error> {
  const auto fd = ::open(bus_path.c_str(), O_RDWR);  // NOLINT(cppcoreguidelines-pro-type-vararg)
  if (fd < 0) {
    const auto err = std::error_code(errno, std::generic_category());
    return std::unexpected(Error{ std::format("(::open({})) ", bus_path.string()), err.message() });
  }
  return I2CBus(fd);
}

//-------------------------------------------------------------------------------------------------
auto I2CBus::write(DevAddr dev_addr, RegAddr reg, std::span<const std::uint8_t> data) const
    -> std::expected<void, Error> {
  static constexpr auto MAX_DATA_LEN = 32UL;
  const auto data_sz = data.size();
  if (data_sz > MAX_DATA_LEN) {
    return std::unexpected(Error{ std::format("Data too long ({}>{})", data_sz, MAX_DATA_LEN) });
  }
  auto buf = std::array<std::uint8_t, MAX_DATA_LEN + 1>{};
  buf[0] = static_cast<std::uint8_t>(reg);
  std::ranges::copy(data, std::next(buf.begin()));
  auto msg = i2c_msg{ .addr = static_cast<std::uint16_t>(dev_addr),
                      .flags = 0,
                      .len = static_cast<std::uint16_t>(1U + data_sz),
                      .buf = buf.data() };
  auto rdwr = i2c_rdwr_ioctl_data{ .msgs = &msg, .nmsgs = 1 };
  if (::ioctl(fd_, I2C_RDWR, &rdwr) < 0) {  // NOLINT(cppcoreguidelines-pro-type-vararg)
    const auto err = std::error_code(errno, std::generic_category());
    return std::unexpected(Error{ "(ioctl(I2C_RDWR)) ", err.message() });
  }
  return {};
}

//-------------------------------------------------------------------------------------------------
auto I2CBus::read(DevAddr dev_addr, RegAddr reg, std::span<std::uint8_t> data) const
    -> std::expected<void, Error> {
  static constexpr auto MAX_DATA_LEN = 32UL;
  const auto data_sz = data.size();
  if (data_sz > MAX_DATA_LEN) {
    return std::unexpected(Error{ std::format("Data too long ({}>{})", data_sz, MAX_DATA_LEN) });
  }
  auto reg_buf = static_cast<std::uint8_t>(reg);
  auto msgs = std::array<struct i2c_msg, 2>{};
  msgs[0] = { .addr = static_cast<std::uint16_t>(dev_addr), .flags = 0, .len = 1, .buf = &reg_buf };
  msgs[1] = { .addr = static_cast<std::uint16_t>(dev_addr),
              .flags = I2C_M_RD,
              .len = static_cast<std::uint16_t>(data_sz),
              .buf = data.data() };
  auto rdwr = i2c_rdwr_ioctl_data{ .msgs = msgs.data(), .nmsgs = 2 };
  if (::ioctl(fd_, I2C_RDWR, &rdwr) < 0) {  // NOLINT(cppcoreguidelines-pro-type-vararg)
    const auto err = std::error_code(errno, std::generic_category());
    return std::unexpected(Error{ "(ioctl(I2C_RDWR) ", err.message() });
  }
  return {};
}

//-------------------------------------------------------------------------------------------------
void I2CBus::close() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

//-------------------------------------------------------------------------------------------------
I2CBus::~I2CBus() {
  close();
}

//-------------------------------------------------------------------------------------------------
I2CBus::I2CBus(int fd) : fd_(fd) {
}

//-------------------------------------------------------------------------------------------------
I2CBus::I2CBus(I2CBus&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {
}

//-------------------------------------------------------------------------------------------------
auto I2CBus::operator=(I2CBus&& other) noexcept -> I2CBus& {
  if (this != &other) {
    close();
    fd_ = std::exchange(other.fd_, -1);
  }
  return *this;
}

}  // namespace grape::rpi::sense_hat
