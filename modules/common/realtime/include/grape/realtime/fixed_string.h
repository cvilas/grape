//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <format>
#include <string_view>

namespace grape::realtime {

//=================================================================================================
/// Fixed size string
///
/// This is an alternative to std::string for low latency applications. The implementation avoids
/// memory allocation, and can be used with std::string_view. See examples and tests for usage.
///
/// @tparam CharT Character type. Some examples are char, wchar_t, char8_t, char16_t, char32_t
/// @tparam Traits Traits class for the character type
/// @tparam N The maximum length of the string excluding the terminating null character
template <typename CharT, std::size_t N, typename Traits = std::char_traits<CharT>>
class BasicFixedString {
public:
  constexpr BasicFixedString() noexcept {
    data_.fill('\0');
  }

  template <std::size_t M>
  // NOLINTNEXTLINE(google-explicit-constructor,cppcoreguidelines-avoid-c-arrays)
  consteval BasicFixedString(const CharT (&str)[M]) {
    const auto length = std::min(M - 1, N);
    Traits::copy(data_.data(), str, length);
    data_.at(length) = '\0';
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr BasicFixedString(const CharT* str) {
    const auto length = std::min(Traits::length(str), N);
    Traits::copy(data_.data(), str, length);
    data_.at(length) = '\0';
  }

  /// Constructs a formatted string
  /// @param fmt format string (see std::format)
  /// @param args Arguments to format
  template <typename... Args>
  explicit BasicFixedString(const std::format_string<Args...> fmt, Args&&... args) {
    const auto result = std::format_to_n(data_.data(), N, fmt, std::forward<Args>(args)...);
    data_.at(std::min(N, static_cast<std::size_t>(result.size))) = '\0';
  }

  [[nodiscard]] constexpr auto cStr() const noexcept -> const CharT* {
    return data_.data();
  }

  [[nodiscard]] constexpr auto data() const noexcept -> const CharT* {
    return data_.data();
  }

  [[nodiscard]] constexpr auto data() noexcept -> CharT* {
    return data_.data();
  }

  [[nodiscard]] constexpr auto str() const noexcept -> std::basic_string_view<CharT, Traits> {
    return std::basic_string_view<CharT, Traits>(data_.data(), length());
  }

  [[nodiscard]] constexpr auto length() const noexcept {
    return Traits::length(data_.data());
  }

  [[nodiscard]] constexpr auto maxSize() const noexcept {
    return N;
  }

  [[nodiscard]] constexpr auto empty() const noexcept {
    return (length() == 0);
  }

  constexpr void clear() noexcept {
    data_.fill('\0');
  }

  constexpr void append(const CharT* str) {
    append(str, Traits::length(str));
  }

  constexpr void append(const CharT* str, std::size_t len) {
    const auto current_length = length();
    const auto length_to_copy = std::min(len, (N - current_length));
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(str, str + length_to_copy, data_.data() + current_length);
    data_.at(current_length + length_to_copy) = '\0';
  }

  /// implemented as a member to avoid implicit conversion
  [[nodiscard]] constexpr auto operator==(const BasicFixedString& rhs) const -> bool {
    return (data_ == rhs.data_);
  }

  [[nodiscard]] constexpr auto operator!=(const BasicFixedString& rhs) const -> bool {
    return !(*this == rhs);
  }

  constexpr void swap(BasicFixedString& rhs) noexcept {
    std::swap(data_, rhs.data_);
  }

private:
  static_assert(N > 0, "Container capacity must be valid");
  std::array<CharT, N + 1> data_{};
};

template <class CharT, std::size_t N, class Traits = std::char_traits<CharT>>
constexpr void swap(const BasicFixedString<CharT, N>& lhs,
                    const BasicFixedString<CharT, N>& rhs) noexcept {
  rhs.swap(lhs);
}

template <std::size_t max_length>
using FixedString = BasicFixedString<char, max_length>;

}  // namespace grape::realtime
