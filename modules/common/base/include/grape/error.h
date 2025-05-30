//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <concepts>
#include <source_location>
#include <string_view>

namespace grape {

template <typename... Strings>
  requires(std::convertible_to<Strings, std::string_view> && ...)
class Error {
public:
  constexpr explicit Error(Strings&&... strings,
                           const std::source_location& loc = std::source_location::current())
    : location_(loc) {
    std::size_t pos = 0;
    const auto append = [&](std::string_view str) {
      const auto num_chars = std::min(str.size(), MAX_MESSAGE_LENGTH - pos);
      std::copy_n(str.begin(), num_chars, message_.begin() + pos);
      pos += num_chars;
    };
    (append(strings), ...);
    message_length_ = pos;
  }

  [[nodiscard]] constexpr auto message() const -> std::string_view {
    return { message_.data(), message_length_ };
  }

  [[nodiscard]] constexpr auto location() const -> const std::source_location& {
    return location_;
  }

private:
  static constexpr auto MAX_MESSAGE_LENGTH = 128U;
  std::size_t message_length_{};
  std::array<char, MAX_MESSAGE_LENGTH> message_{};
  std::source_location location_;
};

template <typename... Strings>
Error(Strings&&...) -> Error<Strings...>;

}  // namespace grape

// TODO:
// - print function like Exception, but print to specified stream
// - clean up example
// - document this class
// - use it in realtime
// - fix warnings