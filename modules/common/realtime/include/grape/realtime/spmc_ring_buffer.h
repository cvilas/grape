//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <span>
#include <string>

#include "grape/error.h"
#include "grape/utils/enums.h"

//=================================================================================================
/// Single-producer multi-consumer lock-free ring buffer over shared memory.
///
/// - The Writer creates and owns the ring buffer; destroying the Writer destroys the buffer.
/// - Only a single Writer is allowed at a time. Create fails if a Writer already exists.
/// - The Writer always succeeds: it overwrites the oldest unread frame when the buffer is full.
/// - Multiple independent Readers are supported, each tracking their own read position.
/// - Readers can reside in different processes on the same host.
/// - A Reader that falls too far behind will have frames dropped to catch up.
//=================================================================================================

namespace grape::spmc_ring_buffer {

//-------------------------------------------------------------------------------------------------
/// Buffer configuration options
struct Config {
  std::size_t frame_length{ 1U };  //!< Length of a single frame in bytes
  std::size_t num_frames{ 1U };    //!< Number of frames in the buffer
};

//=================================================================================================
/// Connects to an existing ring buffer to read data.
class Reader {
public:
  enum class Status : std::uint8_t {
    Empty,    //!< No unread frames available
    Dropped,  //!< Reader fell too far behind; one or more frames were dropped to catch up.
    Ok,       //!< Frame read successfully
  };

  /// Check if a buffer exists
  /// @param name Identifying name of the buffer
  /// @return true if buffer exists
  [[nodiscard]] static auto exists(std::string_view name) -> bool;

  /// Open an existing buffer for reading.
  /// @param name Identifying name of the buffer
  /// @return A Reader on success, or an error if the buffer does not exist or is incompatible
  [[nodiscard]] static auto connect(std::string_view name) -> std::expected<Reader, Error>;

  /// @return configuration parameters
  [[nodiscard]] auto config() const -> Config;

  /// reader function signature
  using ReaderFunc = std::function<void(std::span<const std::byte>)>;

  /// Attempt to read the next unread frame in-place.
  ///
  /// If the writer has lapped this reader, the reader drops frames and fast-forwards to latest
  /// data. The data in this case should be assumed to be invalid. The next call will attempt to
  /// read from the new position.
  ///
  /// @param func Called with a read-only view of the frame.
  /// @return read status
  /// @note Must not be called concurrently.
  [[nodiscard]] auto visit(const ReaderFunc& func) -> Status;

  ~Reader();
  Reader(Reader&& other) noexcept = default;
  auto operator=(Reader&& other) noexcept -> Reader& = default;
  Reader(const Reader&) = delete;
  auto operator=(const Reader&) -> Reader& = delete;

private:
  struct Impl;
  explicit Reader(std::unique_ptr<Impl> impl);
  std::unique_ptr<Impl> impl_;
  std::uint64_t read_count_{};
};

//=================================================================================================
/// Creates a ring buffer and provides methods to write data to it
///
class Writer {
public:
  /// Creates ring buffer and returns buffer writer. Method fails if a writer already exists
  /// @param name A uniquely identifying name for the ring buffer
  /// @param config Buffer configuration parameters
  /// @return A writer to write data into buffer, or an error message
  [[nodiscard]] static auto create(std::string_view name, const Config& config)
      -> std::expected<Writer, Error>;

  /// writer function signature
  using WriterFunc = std::function<void(std::span<std::byte>)>;

  /// Write a frame in-place.
  /// @param func Writing function.
  /// @note Must not be called concurrently.
  void visit(const WriterFunc& func);

  ~Writer();
  Writer(Writer&& other) noexcept = default;
  auto operator=(Writer&& other) noexcept -> Writer& = default;
  Writer(const Writer&) = delete;
  auto operator=(const Writer&) -> Writer& = delete;

private:
  struct Impl;
  explicit Writer(std::string name, std::unique_ptr<Impl> impl);
  std::string name_;
  std::unique_ptr<Impl> impl_;
};

[[nodiscard]] constexpr auto toString(Reader::Status st) -> std::string_view {
  return enums::name(st);
}

}  // namespace grape::spmc_ring_buffer
