//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

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
  /// @param data Pointer to data
  /// @param len Number of bytes to write
  /// @return true on success. false if buffer doesn't have enough space. Nothing is written if so.
  [[nodiscard]] auto write(const char* data, std::size_t len) -> bool {
    if (offset_ + len > MAX_SIZE) {
      return false;
    }
    std::copy_n(data, len, buf_.begin() + offset_);
    offset_ += len;
    return true;
  }

  /// @return Pointer to the stream buffer
  [[nodiscard]] auto data() -> char* {
    return buf_.data();
  }

  /// @return Immutable pointer to the stream buffer
  [[nodiscard]] auto data() const -> const char* {
    return buf_.data();
  }

  /// @return Number of bytes written so far in the stream buffer
  [[nodiscard]] auto size() const -> std::size_t {
    return offset_;
  }

  /// @return maximum number of bytes the stream buffer can hold
  [[nodiscard]] auto capacity() const -> std::size_t {
    return buf_.max_size();
  }

  /// Set next writing position to 'n' bytes behind the current positon
  void rewind(std::size_t n) {
    offset_ = (n < offset_) ? (offset_ - n) : 0u;
  }

  /// Reset next writing position to the beginning of the stream buffer
  void reset() {
    offset_ = 0u;
  }

private:
  std::size_t offset_{ 0 };
  std::array<char, MAX_SIZE> buf_{ 0 };
};

//=================================================================================================
/// A generic interface to a readable fixed-size data stream
class InStream {
public:
  /// Initialise
  /// @param data Serialised data to decode
  explicit InStream(std::span<const char> data) : stream_(data) {
  }

  /// Reads bytes into user-provided location
  /// @param to Pointer to user-provided stream
  /// @param len Number of bytes to read
  /// @return true on success, false if stream doesn't contain the requested number of bytes.
  /// Nothing is read if so.
  [[nodiscard]] auto read(char* const to, std::size_t len) -> bool {
    if (offset_ + len > stream_.size()) {
      return false;
    }
    std::copy_n(stream_.data() + offset_, len, to);
    offset_ += len;
    return true;
  }

  /// @return Number of bytes in the stream buffer
  [[nodiscard]] auto size() const -> std::size_t {
    return stream_.size_bytes();
  }

  /// Set next reading position to 'n' bytes behind the current positon
  void rewind(std::size_t n) {
    offset_ = (n < offset_) ? (offset_ - n) : 0u;
  }

  /// Reset next reading position to the beginning of the stream buffer
  void reset() {
    offset_ = 0u;
  }

private:
  std::size_t offset_{ 0 };
  std::span<const char> stream_;
};

}  // namespace grape::serdes
