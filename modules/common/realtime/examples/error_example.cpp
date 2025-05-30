//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <expected>
#include <print>
#include <system_error>

#include "grape/realtime/error.h"

namespace {

//=================================================================================================
// An example function that returns an error on failure
auto doWork() -> std::expected<void, grape::realtime::Error> {
  const auto err = std::make_error_code(std::errc::invalid_argument);
  return std::unexpected(grape::realtime::Error{ err.message() });
}
}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    const auto result = doWork();
    if (not result) {
      const auto& err = result.error();
      const auto& loc = err.location();
      std::println("[{} ({})] : {}", loc.function_name(), loc.line(), err.message());
    }
    return EXIT_SUCCESS;
  } catch (...) {
    (void)fputs("An exception occured\n", stderr);
    return EXIT_FAILURE;
  }
}
