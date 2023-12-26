//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include "grape/log/logger.h"

namespace grape::log {

//=================================================================================================
// singleton logger. See Logger class for documentation of exposed functions
class SingletonLogger {
public:
  static auto instance() -> Logger& {
    static Logger instance;
    return instance;
  }
  SingletonLogger(const SingletonLogger&) = delete;
  SingletonLogger(const SingletonLogger&&) = delete;
  auto operator=(const SingletonLogger&) -> SingletonLogger& = delete;
  auto operator=(const SingletonLogger&&) -> SingletonLogger& = delete;

private:
  SingletonLogger() = default;
  ~SingletonLogger() = default;
};

auto setStreamBuffer(std::streambuf* buf) -> std::streambuf* {
  return Logger::setStreamBuffer(buf);
}

void setFormatter(Formatter&& f) {
  SingletonLogger::instance().setFormatter(std::move(f));
}

void setThreshold(Severity s) {
  SingletonLogger::instance().setThreshold(s);
}

[[nodiscard]] auto getThreshold() -> Severity {
  return SingletonLogger::instance().getThreshold();
}

[[nodiscard]] auto canLog(Severity severity) -> bool {
  return SingletonLogger::instance().canLog(severity);
}

void log(
    Severity s, const std::string& msg,
    const std::chrono::time_point<std::chrono::system_clock>& tp = std::chrono::system_clock::now(),
    const std::source_location& loc = std::source_location::current()) {
  return SingletonLogger::instance().log(s, msg, tp, loc);
}

}  // namespace grape::log
