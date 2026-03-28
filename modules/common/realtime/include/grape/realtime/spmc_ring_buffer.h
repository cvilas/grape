//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <expected>
#include <functional>
#include <memory>
#include <span>
#include <string>

#include "grape/error.h"

//=================================================================================================
/// single-producer multi-consumer ring buffer
/// - multiple independent readers with their own read pointers. Can reside in different processes
/// - single writer, enforced by design (createWriter fails if a writer already exists with the same
/// name)
/// - writer can lap the reader. reader will just lose data (but will report number of items lost)
/// - underlying data sharing mechanism uses shared memory
//=================================================================================================

namespace grape::spmc_ring_buffer {

/// Buffer configuration options
struct Config {
  std::size_t frame_length;  //!< Length of a single frame in bytes
  std::size_t num_frames;    //!< Number of frames in the buffer
};

/// - only opens and closes buffer
/// - does not modify buffer
class Reader {
public:
  // should be interruptible
  [[nodiscard]] static auto waitForWriter(const std::chrono::milliseconds timeout) -> bool;

  [[nodiscard]] static auto connect(std::string_view name) -> std::expected<Reader, Error>;

  [[nodiscard]] auto config() const -> Config;

  using ReaderFunc = std::function<void(std::span<const std::byte>)>;

  /// Read a frame in-place
  /// @param func Reading function
  /// @return false if buffer has no more items to read, else true
  [[nodiscard]] auto visit(const ReaderFunc& func) -> bool;

  ~Reader();
  Reader(Reader&& other) noexcept;
  auto operator=(Reader&& other) noexcept -> Reader&;
  Reader(const Reader&) = delete;
  auto operator=(const Reader&) -> Reader& = delete;

private:
  Reader();
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

/// - creates and destroys buffer
/// - take unique ownership. don't allow multiple writers
class Writer {
public:
  [[nodiscard]] static auto create(std::string_view name, const Config& config)
      -> std::expected<Writer, Error>;

  using WriterFunc = std::function<void(std::span<std::byte>)>;

  /// Write a frame in-place
  /// @param func Writing function
  void visit(const WriterFunc& func);

  ~Writer();
  Writer(Writer&& other) noexcept;
  auto operator=(Writer&& other) noexcept -> Writer&;
  Writer(const Writer&) = delete;
  auto operator=(const Writer&) -> Writer& = delete;

private:
  Writer();
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace grape::spmc_ring_buffer