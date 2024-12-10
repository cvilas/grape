//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

namespace grape::probe {

/// Lists datatypes that are supported
template <typename T>
struct IsNumeric : std::disjunction<                    //
                       std::is_same<T, std::int8_t>,    //
                       std::is_same<T, std::uint8_t>,   //
                       std::is_same<T, std::int16_t>,   //
                       std::is_same<T, std::uint16_t>,  //
                       std::is_same<T, std::int32_t>,   //
                       std::is_same<T, std::uint32_t>,  //
                       std::is_same<T, std::int64_t>,   //
                       std::is_same<T, std::uint64_t>,  //
                       std::is_same<T, float>,          //
                       std::is_same<T, double>          //
                       > {};

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 2 * sizeof(float));

template <typename T>
constexpr bool IS_NUMERIC_V = IsNumeric<T>::value;

template <typename T>
concept NumericType = IS_NUMERIC_V<T>;

/// Identifiers for supported numeric data types
enum class TypeId : std::uint8_t {
  Int8,
  Uint8,
  Int16,
  Uint16,
  Int32,
  Uint32,
  Int64,
  Uint64,
  Float32,
  Float64,
};

/// @return Type identifier given numeric data type
template <NumericType T>
constexpr auto toTypeId() -> TypeId {
  // clang-format off
  if constexpr (std::is_same_v<T, std::int8_t>) { return TypeId::Int8; }
  if constexpr (std::is_same_v<T, std::uint8_t>) { return TypeId::Uint8; }
  if constexpr (std::is_same_v<T, std::int16_t>) { return TypeId::Int16; }
  if constexpr (std::is_same_v<T, std::uint16_t>) { return TypeId::Uint16; }
  if constexpr (std::is_same_v<T, std::int32_t>) { return TypeId::Int32; }
  if constexpr (std::is_same_v<T, std::uint32_t>) { return TypeId::Uint32; }
  if constexpr (std::is_same_v<T, std::int64_t>) { return TypeId::Int64; }
  if constexpr (std::is_same_v<T, std::uint64_t>) { return TypeId::Uint64; }
  if constexpr (std::is_same_v<T, float>) { return TypeId::Float32; }
  if constexpr (std::is_same_v<T, double>) { return TypeId::Float64; }
  // clang-format on
}

/// @return Length in bytes, for an element of the specified type
constexpr auto length(TypeId tid) -> std::size_t {
  switch (tid) {
      // clang-format off
    case TypeId::Int8: return sizeof(std::int8_t); break;
    case TypeId::Uint8:return sizeof(std::uint8_t); break;
    case TypeId::Int16:return sizeof(std::int16_t); break;
    case TypeId::Uint16:return sizeof(std::uint16_t); break;
    case TypeId::Int32:return sizeof(std::int32_t); break;
    case TypeId::Uint32:return sizeof(std::uint32_t); break;
    case TypeId::Int64:return sizeof(std::int64_t); break;
    case TypeId::Uint64:return sizeof(std::uint64_t); break;
    case TypeId::Float32:return sizeof(float); break;
    case TypeId::Float64:return sizeof(double); break;
      // clang-format on
  }
  std::unreachable();
}
}  // namespace grape::probe
