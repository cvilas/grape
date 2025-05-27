//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <type_traits>

namespace grape::joystick {
template <typename T>
  requires std::is_arithmetic_v<T>
struct [[nodiscard]] Range {
  T minimum{};  //!< bottom end of the range
  T maximum{};  //!< top end of the range
  T fuzz{};     //!< noise band. Events may be ignored if value changes within this band
  T flat{};     //!< deadband around zero (values discarded or reported as 0)
};
}  // namespace grape::joystick