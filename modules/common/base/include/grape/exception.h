//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
// MIT License
//=================================================================================================

#pragma once

#include <source_location>
#include <stdexcept>

namespace grape {
/// Base class for exceptions
/// \include exception_example.cpp
class Exception : public std::runtime_error {
public:
  explicit Exception(const std::string& message,
                     std::source_location location = std::source_location::current());
};

}  // namespace grape