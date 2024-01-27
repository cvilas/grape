//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>
#include <format>

#include "grape/log/record.h"

namespace grape::log {

/// Default log formatter implementation.
inline auto defaultFormatter(const Record& r) -> std::string {
  const auto file_name = std::filesystem::path(r.location.file_name()).filename().string();
  return std::format("[{}] [{}] [{}] [{}:{}] {}", r.timestamp, r.logger_name, toString(r.severity),
                     file_name, r.location.line(), r.message);
}

}  // namespace grape::log
