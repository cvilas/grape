//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <expected>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <type_traits>

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
  static constexpr auto MIN_FRAME_LENGTH = 1U;
  static constexpr auto MIN_FRAMES = 2U;
  std::size_t frame_length{ MIN_FRAME_LENGTH };  //!< Length of a single frame in bytes
  std::size_t num_frames{ MIN_FRAMES };          //!< Number of frames in the buffer
};

//=================================================================================================
/// Connects to an existing ring buffer to read data.
class Reader {
public:
  /// Read operation policy
  enum class Policy : std::uint8_t {
    Next,    //!< Read the next unread frame
    Latest,  //!< Read the latest frame, skipping any intermediate frames
  };

  /// Read operation status codes
  enum class Status : std::uint8_t {
    Empty,     //!< No unread frames
    Dropped,   //!< Reader fell too far behind; frames dropped to catch up. No valid data returned
    Canceled,  //!< Read function returned false. No valid data returned
    Ok,        //!< Frame read successfully
  };

  /// Check if a buffer exists
  /// @param name Identifying name of the buffer
  /// @return true if buffer exists
  [[nodiscard]] static auto exists(std::string_view name) -> bool;

  /// Open an existing buffer for reading.
  /// @param name Identifying name of the buffer
  /// @return A Reader on success, or an error if the buffer does not exist or is incompatible
  [[nodiscard]] static auto connect(std::string_view name) -> std::expected<Reader, Error>;

  /// @return buffer configuration parameters
  [[nodiscard]] auto config() const -> Config;

  /// Attempt to read the next unread frame in-place.
  ///
  /// If the writer has lapped this reader, the reader drops frames and fast-forwards to latest
  /// data. The data in this case should be assumed to be invalid. The next call will attempt to
  /// read from the new position.
  ///
  /// @param fn User-defined callable invoked with a read-only view of the frame.
  ///           Callbable function signature `fn(std::span<const std::byte>) -> bool`.
  ///           Return `false` to abort the read without advancing the read counter.
  /// @param policy Policy to apply for reading
  /// @return read status
  /// @note Must not be called concurrently.
  template <typename F>
    requires std::is_invocable_r_v<bool, F, std::span<const std::byte>>
  [[nodiscard]] auto visit(F&& fn, Policy policy = Policy::Next) -> Status;

  ~Reader();
  Reader(Reader&& other) noexcept = default;
  auto operator=(Reader&& other) noexcept -> Reader& = default;
  Reader(const Reader&) = delete;
  auto operator=(const Reader&) -> Reader& = delete;

private:
  struct Impl;
  explicit Reader(std::unique_ptr<Impl> impl, const Config& config,
                  std::span<const std::byte> frames, const std::uint64_t* write_count_ptr);
  std::unique_ptr<Impl> impl_;
  Config config_;
  std::span<const std::byte> frames_;
  const std::uint64_t* write_count_ptr_{};
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

  /// Write a frame in-place.
  ///
  /// @param fn User-defined callable invoked with a writable view of the frame.
  ///           Callable function signature: `fn(std::span<std::byte>) -> bool`.
  ///           Return `false` to abort the write without advancing the write counter.
  /// @note Must not be called concurrently.
  template <typename F>
    requires std::is_invocable_r_v<bool, F, std::span<std::byte>>
  void visit(F&& fn);

  ~Writer();
  Writer(Writer&& other) noexcept = default;
  auto operator=(Writer&& other) noexcept -> Writer& = default;
  Writer(const Writer&) = delete;
  auto operator=(const Writer&) -> Writer& = delete;

private:
  struct Impl;
  explicit Writer(std::string name, std::unique_ptr<Impl> impl, const Config& config,
                  std::span<std::byte> frames, std::uint64_t* write_count_ptr);
  std::string name_;
  std::unique_ptr<Impl> impl_;
  Config config_;
  std::span<std::byte> frames_;
  std::uint64_t* write_count_ptr_{};
};

//-------------------------------------------------------------------------------------------------
template <typename F>
  requires std::is_invocable_r_v<bool, F, std::span<std::byte>>
void Writer::visit(F&& fn) {
  const auto wc_ref = std::atomic_ref<std::uint64_t>{ *write_count_ptr_ };
  const auto write_count = wc_ref.load(std::memory_order_relaxed);
  const auto frame_start = (write_count % config_.num_frames) * config_.frame_length;
  if (std::invoke(std::forward<F>(fn), frames_.subspan(frame_start, config_.frame_length))) {
    wc_ref.fetch_add(1U, std::memory_order_release);
  }
}

//-------------------------------------------------------------------------------------------------
template <typename F>
  requires std::is_invocable_r_v<bool, F, std::span<const std::byte>>
[[nodiscard]] auto Reader::visit(F&& fn, Policy policy) -> Status {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
  const auto wc_ref =
      std::atomic_ref<std::uint64_t>{ *const_cast<std::uint64_t*>(write_count_ptr_) };
  // NOLINTEND(cppcoreguidelines-pro-type-const-cast)

  const auto write_count = wc_ref.load(std::memory_order_acquire);

  if (policy == Policy::Latest) {
    read_count_ = ((write_count > 0UL) ? (write_count - 1UL) : 0UL);
  }

  // To read a valid frame, read count should be within the following bounds:
  // (write_count - capacity) < read_count < write_count

  // if writer lapped reader, fast-forward read count and report dropped frames
  const auto read_lag = write_count - read_count_;
  const auto frame_good = ((read_lag < config_.num_frames) and (read_count_ < write_count));
  if (not frame_good) {
    read_count_ = write_count;
    return (read_lag == 0UL) ? Status::Empty : Status::Dropped;
  }

  // call user function
  const auto frame_start = (read_count_ % config_.num_frames) * config_.frame_length;
  if (not std::invoke(std::forward<F>(fn), frames_.subspan(frame_start, config_.frame_length))) {
    return Status::Canceled;
  }

  // if writer lapped reader while calling user function, invalidate the read
  const auto write_count_post_read = wc_ref.load(std::memory_order_acquire);
  if ((write_count_post_read - read_count_) >= config_.num_frames) {
    read_count_ = write_count_post_read;
    return Status::Dropped;
  }
  read_count_++;
  return Status::Ok;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(Reader::Status st) -> std::string_view {
  return enums::name(st);
}

}  // namespace grape::spmc_ring_buffer
