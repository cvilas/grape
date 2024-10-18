//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <bit>  // for endian check
#include <span>
#include <string>
#include <vector>

#include "grape/serdes/concepts.h"

namespace grape::serdes {

//-------------------------------------------------------------------------------------------------
// Defines the concept of arithmetic data types, including bool and char
template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

// Some preconditions for interoperability
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 2 * sizeof(float));
static_assert(sizeof(std::size_t) == sizeof(std::uint64_t));
static_assert(std::endian::native == std::endian::little);

//=================================================================================================
/// @brief Simple serialiser that just packs bytes into a stream buffer
/// @tparam Stream Writable stream buffer
template <WritableStream Stream>
class Serialiser {
public:
  /// Initialise with a stream buffer
  /// @param stream The output stream buffer to encode data into
  explicit Serialiser(Stream& stream) : stream_(stream) {
    stream_.reset();
  }

  [[nodiscard]] auto pack(const std::string& value) -> bool {
    return packWithSize(std::span<const char>{ value.c_str(), value.size() });
  }

  template <arithmetic T>
  [[nodiscard]] auto pack(const T& value) -> bool {
    return pack(std::span<const T>{ &value, 1u });
  }

  template <arithmetic T>
  [[nodiscard]] auto pack(const std::vector<T>& data) -> bool {
    return packWithSize(std::span<const T>{ data.begin(), data.size() });
  }

  template <arithmetic T, std::size_t N>
  [[nodiscard]] auto pack(const std::array<T, N>& data) -> bool {
    return pack(std::span<const T>{ data.begin(), data.end() });
  }

private:
  template <arithmetic T>
  [[nodiscard]] auto packWithSize(std::span<const T> data) -> bool {
    if (not pack(static_cast<std::uint32_t>(data.size()))) {
      return false;
    }
    if (not pack(data)) {
      stream_.rewind(sizeof(std::uint32_t));  // undo encoding data size
      return false;
    }
    return true;
  }

  template <arithmetic T>
  [[nodiscard]] auto pack(std::span<const T> data) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return stream_.write(reinterpret_cast<const char*>(data.data()), data.size_bytes());
  }

  Stream& stream_;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

//=================================================================================================
/// Deserialises data encoded with Serialiser class
template <ReadableStream Stream>
class Deserialiser {
public:
  /// Initialise
  /// @param data Serialised data to decode
  explicit Deserialiser(Stream& stream) : stream_(stream) {
  }

  [[nodiscard]] auto unpack(std::string& str) -> bool {
    std::uint32_t sz{};
    if (not unpack(sz)) {
      return false;
    }
    str.resize(sz);
    if (not unpack(std::span<char>{ str.data(), sz })) {
      stream_.rewind(sizeof(std::uint32_t));  // undo decoding size
      return false;
    }
    return true;
  }

  template <arithmetic T>
  [[nodiscard]] auto unpack(T& value) -> bool {
    return unpack(std::span<T>{ &value, 1u });
  }

  template <arithmetic T>
  [[nodiscard]] auto unpack(std::vector<T>& data) -> bool {
    std::uint32_t sz{};
    if (not unpack(sz)) {
      return false;
    }
    data.resize(sz);
    if (not unpack(std::span<T>{ data.begin(), sz })) {
      stream_.rewind(sizeof(std::uint32_t));  // undo decoding size
      return false;
    }
    return true;
  }

  template <arithmetic T, std::size_t N>
  [[nodiscard]] auto unpack(std::array<T, N>& data) -> bool {
    return unpack(std::span<T>{ data.begin(), data.end() });
  }

private:
  template <arithmetic T>
  [[nodiscard]] auto unpack(std::span<T> data) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return stream_.read(reinterpret_cast<char*>(data.data()), data.size_bytes());
  }

  Stream& stream_;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

}  // namespace grape::serdes
