//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <sstream>
#include <vector>

namespace grape::conio {

//=================================================================================================
/// Type trait to detect std::vector types
template <typename T>
struct IsStdVector : std::false_type {};

template <typename T>
struct IsStdVector<std::vector<T>> : std::true_type {};

template <typename T>
constexpr bool IS_STD_VECTOR_V = IsStdVector<T>::value;

//=================================================================================================
/// Basic string-streamable types
template <typename T>
concept BasicStringStreamable = requires(std::string str, T value) {
  std::istringstream{ str } >> value;
  std::ostringstream{ str } << value;
};

//=================================================================================================
/// Types that are convertible to and from a string
template <typename T>
concept StringStreamable = BasicStringStreamable<T> ||
                           (IS_STD_VECTOR_V<T> && BasicStringStreamable<typename T::value_type>);

//=================================================================================================
/// Input stream operator for std::vector<T> - reads comma-separated values
template <BasicStringStreamable T>
auto operator>>(std::istream& is, std::vector<T>& vec) -> std::istream& {
  vec.clear();
  if (std::string line; std::getline(is, line)) {
    auto iss = std::istringstream(line);
    auto token = std::string{};
    while (std::getline(iss, token, ',')) {
      token.erase(0, token.find_first_not_of(" [\t\n\r"));
      token.erase(token.find_last_not_of(" ]\t\n\r") + 1);
      auto value_stream = std::istringstream(token);
      T value;
      value_stream >> value;
      vec.emplace_back(std::move(value));
    }
  }
  return is;
}

//=================================================================================================
/// Output stream operator for std::vector<T> - writes comma-separated values
template <BasicStringStreamable T>
auto operator<<(std::ostream& os, const std::vector<T>& vec) -> std::ostream& {
  if (auto it = vec.begin(); it != vec.end()) {
    os << *it;
    for (++it; it != vec.end(); ++it) {
      os << "," << *it;
    }
  }
  return os;
}
}  // namespace grape::conio
