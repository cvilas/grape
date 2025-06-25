//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "grape/serdes/concepts.h"

namespace grape::serdes {

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
    return pack(std::span<const T>{ &value, 1U });
  }

  template <arithmetic T>
  [[nodiscard]] auto pack(const std::vector<T>& data) -> bool {
    return packWithSize(std::span<const T>{ data.data(), data.size() });
  }

  template <arithmetic T, std::size_t N>
  [[nodiscard]] auto pack(const std::array<T, N>& data) -> bool {
    return pack(std::span<const T>{ data.data(), data.size() });
  }

  template <typename... Types>
  [[nodiscard]] auto pack(const std::variant<Types...>& value) -> bool {
    if (not this->pack(value.index())) {
      return false;
    }
    return std::visit([this](const auto& val) { return this->pack(val); }, value);
  }

  template <typename T>
    requires Serialisable<T, Stream>
  [[nodiscard]] auto pack(const T& value) -> bool {
    return serialise(*this, value);  // call user-defined function
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
  /// @param stream Serialised data to decode
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
    return unpack(std::span<T>{ &value, 1U });
  }

  template <arithmetic T>
  [[nodiscard]] auto unpack(std::vector<T>& data) -> bool {
    std::uint32_t sz{};
    if (not unpack(sz)) {
      return false;
    }
    data.resize(sz);
    if (not unpack(std::span<T>{ data.data(), sz })) {
      stream_.rewind(sizeof(std::uint32_t));  // undo decoding size
      return false;
    }
    return true;
  }

  template <arithmetic T, std::size_t N>
  [[nodiscard]] auto unpack(std::array<T, N>& data) -> bool {
    return unpack(std::span<T>{ data.data(), data.size() });
  }

  template <typename... Types>
  [[nodiscard]] auto unpack(std::variant<Types...>& value) -> bool {
    auto idx = std::size_t{};
    if (not this->unpack(idx)) {
      return false;
    }

    if (idx >= sizeof...(Types)) {
      return false;  // invalid index
    }

    bool success = false;

    // Helper to unpack at a specific index
    auto unpack_at_index = [&]<std::size_t I>() {
      if (idx == I) {
        using T = std::variant_alternative_t<I, std::variant<Types...>>;
        T val{};
        if (this->unpack(val)) {
          value = std::move(val);
          success = true;
        }
      }
    };

    // Try to unpack at each possible index
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      (unpack_at_index.template operator()<Is>(), ...);
    }(std::make_index_sequence<sizeof...(Types)>{});

    return success;
  }

  template <typename T>
    requires Deserialisable<T, Stream>
  [[nodiscard]] auto unpack(T& value) -> bool {
    return deserialise(*this, value);  // call user-defined function
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
