//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <source_location>
#include <stdexcept>
#include <string>

#include "grape/utils/stacktrace.h"

namespace grape {

//=================================================================================================
/// Base class interface for exceptions
///
/// Guidelines:
/// - DO USE exceptions to log and trace terminal errors before exit
/// - DO define as few exception types as possible
/// - DO NOT USE exceptions for resource management (eg: to release resources). Use RAII instead
/// - DO NOT USE exceptions for loop control. Use return codes, std::optional and std::expected
/// - DO NOT USE exceptions for memory corruption or exhaustion. Terminate instead.
///
/// @todo Integrate std::stacktrace when compiler support becomes available
/// @include exception_example.cpp
class Exception : std::runtime_error {
public:
  /// @param message A context-specific description of the error
  /// @param location Source location where the error was triggered
  /// @param trace Stack trace leading to error
  Exception(const std::string& message, const std::source_location& location,
            utils::StackTrace trace)
    : std::runtime_error{ message }, location_{ location }, trace_{ std::move(trace) } {
  }

  /// @return Location in the source where the exception occurred
  [[nodiscard]] auto location() const noexcept -> const std::source_location& {
    return location_;
  }

  /// @return Backtrace leading up to exception
  [[nodiscard]] auto trace() const noexcept -> const utils::StackTrace& {
    return trace_;
  }

  /// Lippincott function usable in catch(..) block to report diagnostics from exception thrown
  static void print() noexcept;

private:
  std::source_location location_;
  utils::StackTrace trace_;
};

/// User function to throw an exception
template <typename T>
  requires std::derived_from<T, Exception>
constexpr void panic(const std::string& message,
                     const std::source_location& location = std::source_location::current(),
                     utils::StackTrace trace = utils::StackTrace::current()) {
  throw T{ message, location, std::move(trace) };
}

}  // namespace grape
