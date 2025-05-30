//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <source_location>

#include "grape/realtime/fixed_string.h"

namespace grape::realtime {

//=================================================================================================
/// A fixed-size encapsulation for error messages. Suitable for use in real-time context.
///
/// See example programs for usage
class Error {
public:
  /// Constructs
  /// @param strings List of strings concatenated into internal message buffer. The message is
  /// truncated if total length exceeds buffer space.
  /// @param location Source location. Default: Location where the Error was constructed
  constexpr Error(std::initializer_list<std::string_view> strings,
                  const std::source_location& location = std::source_location::current())
    : location_(location) {
    for (const auto& str : strings) {
      message_.append(str.data(), str.length());
    }
  }

  /// @return error message
  [[nodiscard]] constexpr auto message() const -> std::string_view {
    return message_.str();
  }

  /// @return source location captured at construction
  [[nodiscard]] constexpr auto location() const -> const std::source_location& {
    return location_;
  }

private:
  static constexpr auto MAX_MESSAGE_LENGTH = 128UL;
  FixedString<MAX_MESSAGE_LENGTH> message_;
  std::source_location location_;
};

// Checks to ensure suitability in realtime context
static_assert(std::is_trivially_copyable_v<Error>);
static_assert(std::is_nothrow_copy_constructible_v<Error>);
static_assert(std::is_nothrow_move_constructible_v<Error>);
static_assert(std::is_nothrow_copy_assignable_v<Error>);
static_assert(std::is_nothrow_move_assignable_v<Error>);
static_assert(std::is_nothrow_destructible_v<Error>);

}  // namespace grape::realtime
