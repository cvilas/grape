//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <bit>  // for endian check
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>

namespace grape::serdes {

//-------------------------------------------------------------------------------------------------
// Concept for buffer streams
template <typename Stream>
concept WritableStream = requires(Stream strm, std::span<const std::byte> data) {
  { strm.write(data) } -> std::same_as<bool>;
  /* Writes 'data' into stream 's'. Return false on failure  */
};

template <typename Stream>
concept ReadableStream = requires(Stream strm, std::span<std::byte> data) {
  { strm.read(data) } -> std::same_as<bool>;
  /* Reads 'data' from stream 's'. Return false on failure  */
};

template <WritableStream Stream>
class Serialiser;

template <ReadableStream Stream>
class Deserialiser;

//-------------------------------------------------------------------------------------------------
// Concepts for serialization requirements
template <typename T, typename Stream>
concept Serialisable = WritableStream<Stream> && requires(Serialiser<Stream>& ser, const T& value) {
  { serialise(ser, value) } -> std::same_as<bool>;
};

template <typename T, typename Stream>
concept Deserialisable = ReadableStream<Stream> && requires(Deserialiser<Stream>& des, T& value) {
  { deserialise(des, value) } -> std::same_as<bool>;
};

//-------------------------------------------------------------------------------------------------
// Concept for arithmetic data types, including bool and char
template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

// Some preconditions for interoperability
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 2 * sizeof(float));
static_assert(sizeof(std::size_t) == sizeof(std::uint64_t));
static_assert(std::endian::native == std::endian::little);

}  // namespace grape::serdes
