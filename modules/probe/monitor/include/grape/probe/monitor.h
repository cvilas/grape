//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <functional>
#include <memory>
#include <span>
#include <vector>

#include "grape/probe/signal.h"
#include "grape/utils/enums.h"

namespace grape::probe {

// Proof of concept
class Monitor {
public:
  enum class Error : std::uint8_t { Renderer, SignalNotFound, SizeMismatch };

  /// Signature for function to receive log records
  using Sender = std::function<void(const std::string&, std::span<const std::byte>)>;

  Monitor();
  ~Monitor();
  void setSender(Sender&& sender);
  void run();

  // receives a snapshot frame
  void recv(const std::vector<grape::probe::Signal>& signals, std::span<const std::byte> frame);

  Monitor(const Monitor&) = delete;
  auto operator=(const Monitor&) = delete;
  Monitor(Monitor&&) = delete;
  auto operator=(Monitor&&) = delete;

private:
  static void glfwEerrorCb(int error, const char* description);
  void drawPlots();
  void drawControls();
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const Monitor::Error& code) -> std::string_view {
  return enums::name(code);
}

}  // namespace grape::probe
