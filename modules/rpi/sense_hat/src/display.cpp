//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/rpi/sense_hat/display.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "grape/exception.h"

namespace grape::rpi::sense_hat {

namespace {

/// Scan /sys/class/graphics/fbN/name to find the framebuffer owned by the Sense HAT LED matrix
/// driver ("RPi-Sense FB"). Returns the device path (e.g. /dev/fb1) or throws on failure.
auto findSenseHatFramebuffer() -> std::filesystem::path {
  static constexpr auto FB_SYS_PATH = "/sys/class/graphics";
  static constexpr auto FB_NAME = "RPi-Sense FB";

  for (const auto& entry : std::filesystem::directory_iterator(FB_SYS_PATH)) {
    const auto fb_name = entry.path().filename().string();
    if (not fb_name.starts_with("fb")) {
      continue;
    }

    const auto name_path = entry.path() / "name";
    auto fs = std::ifstream(name_path);
    if (!fs) {
      continue;
    }

    auto name = std::string{};
    std::getline(fs, name);
    if (name.find(FB_NAME) != std::string::npos) {
      return std::filesystem::path("/dev") / fb_name;
    }
  }

  panic<Exception>("Sense HAT framebuffer not found");
}

}  // namespace

//-------------------------------------------------------------------------------------------------
Display::Display() {
  const auto dev = findSenseHatFramebuffer();
  const auto fd = ::open(dev.c_str(), O_RDWR);  // NOLINT(cppcoreguidelines-pro-type-vararg)
  if (fd < 0) {
    panic<Exception>(std::format("Cannot open framebuffer device '{}'", dev.string()));
  }
  const auto fb_flags = PROT_READ | PROT_WRITE;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* const fb = reinterpret_cast<RGB565*>(mmap(nullptr, FB_BYTES, fb_flags, MAP_SHARED, fd, 0));
  if (fb == MAP_FAILED) {  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    ::close(fd);
    panic<Exception>(std::format("Failed to mmap framebuffer '{}'", dev.string()));
  }
  ::close(fd);
  frame_buf_ = fb;
}

//-------------------------------------------------------------------------------------------------
Display::~Display() {
  if (frame_buf_ != nullptr) {
    munmap(frame_buf_, FB_BYTES);
  }
}

//-------------------------------------------------------------------------------------------------
void Display::clear() {
  std::memset(frame_buf_, 0, FB_BYTES);
}

//-------------------------------------------------------------------------------------------------
void Display::set(const Coordinate& coord, const RGB565& color) {
  get()[(coord.y * WIDTH) + coord.x] = color;
}

//-------------------------------------------------------------------------------------------------
void Display::set(std::span<const RGB565, NUM_PIXELS> colors) {
  std::ranges::copy(colors, get().begin());
}

//-------------------------------------------------------------------------------------------------
auto Display::get(const Coordinate& coord) const -> RGB565 {
  return get()[(coord.y * WIDTH) + coord.x];
}

//-------------------------------------------------------------------------------------------------
auto Display::get() const -> std::span<const RGB565, NUM_PIXELS> {
  return std::span<const RGB565, NUM_PIXELS>(frame_buf_, NUM_PIXELS);
}

//-------------------------------------------------------------------------------------------------
auto Display::get() -> std::span<RGB565, NUM_PIXELS> {
  return std::span<RGB565, NUM_PIXELS>(frame_buf_, NUM_PIXELS);
}

}  // namespace grape::rpi::sense_hat
