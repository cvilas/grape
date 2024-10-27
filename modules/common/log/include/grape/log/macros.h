//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define GRAPE_LOG(logger, level, fmt, ...)                                                         \
  do {                                                                                             \
    (logger).log(level, std::source_location::current(), fmt, ##__VA_ARGS__);                      \
  } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)
#pragma clang diagnostic pop
