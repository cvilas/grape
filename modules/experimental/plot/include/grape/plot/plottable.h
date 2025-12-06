//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <concepts>
#include <type_traits>

namespace grape {

template <typename T>
concept HasTimestamp = requires(T data) {
  { data.timestamp } -> std::same_as<std::remove_cvref_t<decltype(data.timestamp)>&>;
  requires std::is_same_v<
      std::remove_cvref_t<decltype(data.timestamp)>,
      std::chrono::time_point<typename std::remove_cvref_t<decltype(data.timestamp)>::clock,
                              typename std::remove_cvref_t<decltype(data.timestamp)>::duration>>;
};

template <typename T>
concept HasArithmeticFields = requires {
  requires std::is_aggregate_v<T>;
  requires std::is_standard_layout_v<T>;
};

//=================================================================================================
/// Concept for plottable time series data types
///
/// A Plottable type must:
/// - Contain a timestamp field of any std::chrono::time_point type
/// - Be an aggregate type (struct)
/// - Have standard layout for predictable memory access
/// - Contain fields that are integral or floating point types (arithmetic)
///
/// Example usage:
/// @code
/// struct SensorData {
///   std::chrono::nanoseconds timestamp;
///   double temperature;
///   int32_t pressure;
///   float humidity;
/// };
/// static_assert(Plottable<SensorData>);
/// @endcode
template <typename T>
concept Plottable = HasTimestamp<T> && HasArithmeticFields<T>;

}  // namespace grape
