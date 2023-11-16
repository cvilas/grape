//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "grape/log/log.h"

#include <iostream>

namespace grape::log::detail {

void Logger::log(const Record& record) const {
  if (canLog(record.severity)) {
    std::clog << formatter_(record);
  }
}

auto Logger::setStreamBuffer(std::streambuf* buf) -> std::streambuf* {
  return std::clog.rdbuf(buf);
}

}  // namespace grape::log::detail
