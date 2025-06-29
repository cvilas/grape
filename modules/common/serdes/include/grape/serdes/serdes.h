//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
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
  explicit constexpr Serialiser(Stream& stream) : stream_(stream) {
    stream_.reset();
  }

  [[nodiscard]] constexpr auto pack(const std::string& value) -> bool {
    return packWithSize(std::span<const char>{ value.c_str(), value.size() });
  }

  template <arithmetic T>
  [[nodiscard]] constexpr auto pack(const T& value) -> bool {
    return pack(std::span<const T>{ &value, 1U });
  }

  template <arithmetic T>
  [[nodiscard]] constexpr auto pack(const std::vector<T>& data) -> bool {
    return packWithSize(std::span<const T>{ data.data(), data.size() });
  }

  template <arithmetic T, std::size_t N>
  [[nodiscard]] constexpr auto pack(const std::array<T, N>& data) -> bool {
    return pack(std::span<const T>{ data.data(), data.size() });
  }

  template <typename... Types>
  [[nodiscard]] constexpr auto pack(const std::variant<Types...>& value) -> bool {
    if (not this->pack(value.index())) {
      return false;
    }
    return std::visit([this](const auto& val) { return this->pack(val); }, value);
  }

  template <typename T>
    requires Serialisable<T, Stream>
  [[nodiscard]] constexpr auto pack(const T& value) -> bool {
    return serialise(*this, value);  // call user-defined function
  }

private:
  template <arithmetic T>
  [[nodiscard]] constexpr auto packWithSize(std::span<const T> data) -> bool {
    if (not pack(data.size())) {
      return false;
    }
    if (not pack(data)) {
      stream_.rewind(sizeof(std::size_t));  // undo encoding data size
      return false;
    }
    return true;
  }

  template <arithmetic T>
  [[nodiscard]] constexpr auto pack(std::span<const T> data) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return stream_.write({ reinterpret_cast<const std::byte*>(data.data()), data.size_bytes() });
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
  explicit constexpr Deserialiser(Stream& stream) : stream_(stream) {
  }

  [[nodiscard]] constexpr auto unpack(std::string& str) -> bool {
    std::size_t sz{};
    if (not unpack(sz)) {
      return false;
    }
    str.resize(sz);
    if (not unpack(std::span<char>{ str.data(), sz })) {
      stream_.rewind(sizeof(std::size_t));  // undo decoding size
      return false;
    }
    return true;
  }

  template <arithmetic T>
  [[nodiscard]] constexpr auto unpack(T& value) -> bool {
    return unpack(std::span<T>{ &value, 1U });
  }

  template <arithmetic T>
  [[nodiscard]] constexpr auto unpack(std::vector<T>& data) -> bool {
    std::size_t sz{};
    if (not unpack(sz)) {
      return false;
    }
    data.resize(sz);
    if (not unpack(std::span<T>{ data.data(), sz })) {
      stream_.rewind(sizeof(std::size_t));  // undo decoding size
      return false;
    }
    return true;
  }

  template <arithmetic T, std::size_t N>
  [[nodiscard]] constexpr auto unpack(std::array<T, N>& data) -> bool {
    return unpack(std::span<T>{ data.data(), data.size() });
  }

  template <typename... Types>
  [[nodiscard]] constexpr auto unpack(std::variant<Types...>& value) -> bool {
    std::size_t idx{};
    if (not this->unpack(idx)) {
      return false;
    }

    if (idx >= sizeof...(Types)) {
      return false;
    }

    // O(1) dispatch using constexpr function pointer array (creates a compile-time jump table)
    using variant_type = std::variant<Types...>;
    using unpack_fn = bool (*)(Deserialiser*, variant_type*);

    // Generate function pointer array at compile time - make static for true O(1)
    static constexpr auto DISPATCH_TABLE = []() constexpr {
      std::array<unpack_fn, sizeof...(Types)> table{};

      [&table]<std::size_t... Is>(std::index_sequence<Is...>) constexpr {
        auto make_unpacker = []<std::size_t I>() constexpr -> unpack_fn {
          return [](Deserialiser* self, variant_type* var) -> bool {
            using T = std::variant_alternative_t<I, variant_type>;
            T val{};
            if (self->unpack(val)) {
              *var = std::move(val);
              return true;
            }
            return false;
          };
        };

        ((table[Is] = make_unpacker.template operator()<Is>()), ...);
      }(std::make_index_sequence<sizeof...(Types)>{});

      return table;
    }();

    // dispatch function call to unpack the variant type
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    if (not DISPATCH_TABLE[idx](this, &value)) {
      stream_.rewind(sizeof(std::size_t));
      return false;
    }

    return true;
  }

  template <typename T>
    requires Deserialisable<T, Stream>
  [[nodiscard]] constexpr auto unpack(T& value) -> bool {
    return deserialise(*this, value);  // call user-defined function
  }

private:
  template <arithmetic T>
  [[nodiscard]] constexpr auto unpack(std::span<T> data) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return stream_.read({ reinterpret_cast<std::byte*>(data.data()), data.size_bytes() });
  }

  Stream& stream_;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

}  // namespace grape::serdes
