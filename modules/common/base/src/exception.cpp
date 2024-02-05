//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#pragma once

#include "grape/exception.h"

#include "grape/utils/utils.h"

namespace grape {
void AbstractException::consume() noexcept {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)
  try {
    if (std::current_exception() != nullptr) {
      throw;
    }
  } catch (const AbstractException& ex) {
    const auto& loc = ex.where();
    std::ignore = fprintf(stderr, "%s\nin\n%s\nat\n%s:%d\n", ex.what().c_str(), loc.function_name(),
                          utils::truncate(loc.file_name(), "modules").data(), loc.line());
    std::ignore = fprintf(stderr, "Backtrace:\n");
    for (const auto& s : ex.when().trace()) {
      std::ignore = fprintf(stderr, "%s\n", s.c_str());
    }

  } catch (const std::exception& ex) {
    std::ignore = fprintf(stderr, "Exception: %s\n", ex.what());
  } catch (...) {
    std::ignore = fputs("Unknown exception\n", stderr);
  }
  // NOLINTEND(cppcoreguidelines-pro-type-vararg)
}

}  // namespace grape