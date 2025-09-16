//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#include <algorithm>  // std::copy_n
#include <array>
#include <span>

namespace grape::serdes {

//=================================================================================================
/// @brief A generic interface to writable fixed-size data stream
/// @tparam MAX_SIZE Maximum number of bytes the buffer can hold
template <std::size_t MAX_SIZE>
class OutStream {
public:
  /// @brief Write data into stream buffer
  /// @param data data span to write
  /// @return true on success. false if buffer doesn't have enough space. Nothing is written if so.
  [[nodiscard]] constexpr auto write(std::span<const std::byte> data) -> bool {
    const auto len = data.size_bytes();
    if (offset_ + len > MAX_SIZE) {
      return false;
    }
    auto to = std::span{ buf_ }.subspan(offset_, len);
    std::copy(data.begin(), data.end(), to.begin());
    offset_ += len;
    return true;
  }

  /// @return Pointer to the stream buffer
  [[nodiscard]] constexpr auto data() -> std::span<std::byte> {
    return { buf_.data(), offset_ };
  }

  /// @return Immutable pointer to the stream buffer
  [[nodiscard]] constexpr auto data() const -> std::span<const std::byte> {
    return { buf_.data(), offset_ };
  }

  /// @return Number of bytes written so far in the stream buffer
  [[nodiscard]] constexpr auto size() const -> std::size_t {
    return offset_;
  }

  /// @return maximum number of bytes the stream buffer can hold
  [[nodiscard]] constexpr auto capacity() const -> std::size_t {
    return buf_.max_size();
  }

  /// Set next writing position to 'n' bytes behind the current positon
  constexpr void rewind(std::size_t n) {
    offset_ = (n < offset_) ? (offset_ - n) : 0U;
  }

  /// Reset next writing position to the beginning of the stream buffer
  constexpr void reset() {
    offset_ = 0U;
  }

private:
  std::size_t offset_{ 0 };
  std::array<std::byte, MAX_SIZE> buf_{};
};

//=================================================================================================
/// A generic interface to a readable fixed-size data stream
class InStream {
public:
  /// Initialise
  /// @param data Serialised data to decode
  explicit constexpr InStream(std::span<const std::byte> data) : stream_(data) {
  }

  /// Reads bytes into user-provided location
  /// @param to user-provided stream with implicitly defined number of bytes to read
  /// @return true on success, false if stream doesn't contain the requested number of bytes.
  /// Nothing is read if so.
  [[nodiscard]] constexpr auto read(std::span<std::byte> to) -> bool {
    const auto len = to.size_bytes();
    if (offset_ + len > stream_.size()) {
      return false;
    }
    const auto from = stream_.subspan(offset_, len);
    std::ranges::copy(from, to.begin());
    offset_ += len;
    return true;
  }

  /// @return Number of bytes in the stream buffer
  [[nodiscard]] constexpr auto size() const -> std::size_t {
    return stream_.size_bytes();
  }

  /// Set next reading position to 'n' bytes behind the current positon
  constexpr void rewind(std::size_t n) {
    offset_ = (n < offset_) ? (offset_ - n) : 0U;
  }

  /// Reset next reading position to the beginning of the stream buffer
  constexpr void reset() {
    offset_ = 0U;
  }

private:
  std::size_t offset_{ 0 };
  std::span<const std::byte> stream_;
};

}  // namespace grape::serdes
