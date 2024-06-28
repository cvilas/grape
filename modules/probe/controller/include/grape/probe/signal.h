//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cinttypes>

#include "grape/probe/type_id.h"
#include "grape/realtime/fixed_string.h"

namespace grape::probe {

//=================================================================================================
/// Details of a monitored variable in the application. The variable can be a scalar or a sequence.
struct Signal {
  static_assert(sizeof(std::uintptr_t) == sizeof(std::int64_t), "Only 64-bit arch. supported");
  static_assert(sizeof(std::size_t) == sizeof(std::uint64_t), "Only 64-bit arch. supported");

  /// Describes how the signal is utilised
  enum class Role : std::uint8_t {
    Watch,   //!< logging only (read only)
    Control  //!< loggable and remotely settable (read and write)
  };

  static constexpr auto MAX_NAME_LENGTH = 63;

  realtime::FixedString<MAX_NAME_LENGTH> name;  //!< Unique identifier name for the signal
  std::uintptr_t address{ 0 };    //!< Address of the signal in the process address space
  std::size_t num_elements{ 0 };  //!< Number of elements in the signal sequence
  TypeId type{};                  //!< Data type of elements in the signal sequence
  Role role{};                    //!< Mode of usage of the signal
};

// Ensure any future changes maintains triviality for the sake of performance
static_assert(std::is_trivially_copyable_v<Signal> == true);
static_assert(std::is_trivially_move_constructible_v<Signal> == true);

}  // namespace grape::probe