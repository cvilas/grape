//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#pragma once

#include <source_location>
#include <stdexcept>

namespace grape {

/// Base class for exceptions
/// @include exception_example.cpp
class Exception : public std::runtime_error {
public:
  /// @param message A message describing the error and what caused it
  /// @param location Location in the source where the error was triggered at
  explicit Exception(const std::string& message,
                     std::source_location location = std::source_location::current());
};

}  // namespace grape