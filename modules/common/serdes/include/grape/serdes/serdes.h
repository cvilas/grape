//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "grape/serdes/concepts.h"

namespace grape::serdes {

namespace detail {

/// Concept defines aggregate types on which variadic structured bindings can be applied
template <typename T>
concept SerializableAggregate = std::is_aggregate_v<std::remove_cvref_t<T>> && requires(T&& value) {
  []<typename U>(U & obj) constexpr {
    auto&& [... fields] = obj;
    ((void)fields, ...);
  }(std::forward<T>(value));
};

/// Processes members of an aggregate type T field-by-field using fn
template <SerializableAggregate T>
[[nodiscard]] constexpr auto processMembers(T&& value, auto&& fn) -> bool {
  auto process = std::forward<decltype(fn)>(fn);
  return [&process]<typename U>(U& obj) constexpr -> bool {
    auto&& [... fields] = obj;
    auto ok = true;
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    ((ok = ok && process(fields)), ...);
    return ok;
  }(std::forward<T>(value));
}

}  // namespace detail

//=================================================================================================
/// @brief Simple serialiser that just packs bytes into a stream buffer
/// @tparam Stream Writable stream buffer
template <WritableStream Stream>
class Serialiser {
public:
  /// Initialise with a stream buffer
  /// @param stream The output stream buffer to encode data into
  explicit constexpr Serialiser(Stream& stream) : stream_(stream) {
  }

  [[nodiscard]] constexpr auto pack(const std::string& value) -> bool {
    return packWithSize(std::span<const char>{ value.c_str(), value.size() });
  }

  template <arithmetic T>
  [[nodiscard]] constexpr auto pack(const T& value) -> bool {
    return pack(std::span<const T>{ &value, 1U });
  }

  template <typename T>
    requires std::is_enum_v<T>
  [[nodiscard]] constexpr auto pack(const T& value) -> bool {
    using Underlying = std::underlying_type_t<T>;
    return this->pack(static_cast<Underlying>(value));
  }

  template <typename Rep, typename Period>
  [[nodiscard]] constexpr auto pack(const std::chrono::duration<Rep, Period>& value) -> bool {
    return this->pack(value.count());
  }

  template <typename Clock, typename Duration>
  [[nodiscard]] constexpr auto pack(const std::chrono::time_point<Clock, Duration>& value) -> bool {
    return this->pack(value.time_since_epoch());
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
    requires detail::SerializableAggregate<const T&>
  [[nodiscard]] constexpr auto pack(const T& value) -> bool {
    return detail::processMembers(value, [this](const auto& field) { return this->pack(field); });
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

  template <typename T>
    requires std::is_enum_v<T>
  [[nodiscard]] constexpr auto unpack(T& value) -> bool {
    using Underlying = std::underlying_type_t<T>;
    auto raw = Underlying{};
    if (not this->unpack(raw)) {
      return false;
    }
    value = static_cast<T>(raw);
    return true;
  }

  template <typename Rep, typename Period>
  [[nodiscard]] constexpr auto unpack(std::chrono::duration<Rep, Period>& value) -> bool {
    auto raw = Rep{};
    if (not this->unpack(raw)) {
      return false;
    }
    value = std::chrono::duration<Rep, Period>{ raw };
    return true;
  }

  template <typename Clock, typename Duration>
  [[nodiscard]] constexpr auto unpack(std::chrono::time_point<Clock, Duration>& value) -> bool {
    auto duration = Duration{};
    if (not this->unpack(duration)) {
      return false;
    }
    value = std::chrono::time_point<Clock, Duration>{ duration };
    return true;
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

    using VariantType = std::variant<Types...>;
    using UnpackFn = bool (*)(Deserialiser*, VariantType*);

    // create function pointer array for O(1) dispatch
    static constexpr auto DISPATCH_TABLE =
        []<std::size_t... Is>(std::index_sequence<Is...>) constexpr {
          return std::array<UnpackFn, sizeof...(Types)>{ []<std::size_t I>() constexpr -> UnpackFn {
            return [](Deserialiser* self, VariantType* var) -> bool {
              using T = std::variant_alternative_t<I, VariantType>;
              T val{};
              if (self->unpack(val)) {
                *var = std::move(val);
                return true;
              }
              return false;
            };
          }.template operator()<Is>()... };
        }(std::make_index_sequence<sizeof...(Types)>{});

    // unpack the type at index idx in the variant
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    if (not DISPATCH_TABLE.at(idx)(this, &value)) {
      stream_.rewind(sizeof(std::size_t));
      return false;
    }

    return true;
  }

  template <typename T>
    requires detail::SerializableAggregate<T&>
  [[nodiscard]] constexpr auto unpack(T& value) -> bool {
    return detail::processMembers(value, [this](auto& field) { return this->unpack(field); });
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
