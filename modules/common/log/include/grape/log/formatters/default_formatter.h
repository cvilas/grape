//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <filesystem>
#include <format>

#include "grape/log/record.h"

namespace grape::log {

/// Default log formatter implementation.
struct DefaultFormatter {
  static auto format(const Record& record) -> std::string {
    const auto file_name = std::filesystem::path(record.location.file_name()).filename().string();
    return std::format("[{}] [{}] [{}] [{}:{}] {}", record.timestamp, record.logger_name.cStr(),
                       toString(record.severity), file_name, record.location.line(),
                       record.message.cStr());
  }
};

}  // namespace grape::log
