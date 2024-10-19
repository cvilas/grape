//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <string>
#include <vector>

namespace grape::utils {

//=================================================================================================
/// Captures stack trace (similar to std::stacktrace)
/// @todo Deprecate when compiler support for std::stacktrace becomes available
class StackTrace {
public:
  [[nodiscard]] static auto current() -> StackTrace;
  [[nodiscard]] auto trace() const -> const std::vector<std::string>&;

private:
  static constexpr auto MAX_FRAMES = 16U;
  std::vector<std::string> symbol_list_;
};
}  // namespace grape::utils
