//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <expected>
#include <span>
#include <string>
#include <vector>

#include "grape/exception.h"
#include "grape/probe/signal.h"
#include "grape/realtime/fifo_buffer.h"

namespace grape::probe {

//=================================================================================================
/// Configuration of signals monitored by probe controller
class PinConfig {
public:
  /// Pin a scalar variable for monitoring
  /// @param name An identifier name for the variable
  /// @param var The reference to the variable
  /// @param role whether monitored for logging-only or logging and control
  template <NumericType T>
  auto pin(const std::string& name, const T& var, Signal::Role role) -> PinConfig&;

  /// Pin a sequence variable (eg: vector, array) for monitoring
  /// @param name An identifier name for the variable
  /// @param var The reference to the variable
  /// @param role whether monitored for logging-only or logging and control
  template <NumericType T>
  auto pin(const std::string& name, std::span<const T> var, Signal::Role role) -> PinConfig&;

  /// @return Reference to internal array of pinned signals
  [[nodiscard]] auto signals() const -> const std::vector<Signal>&;

  /// Reorder the signals array by their location in the process address space but partition it to
  /// put watchables first and controllables after.
  /// @return Location of first controllable in the array
  [[nodiscard]] auto sort() -> std::vector<Signal>::const_iterator;

private:
  std::vector<Signal> signals_;
};

//=================================================================================================
// Configuration for probe controller capture buffers
struct BufferConfig {
  /// Capacity of snapshot frame buffer. A frame records the state of all variables from a single
  /// call to Controller::snap().
  std::size_t snap_buffer_capacity{ 0 };

  /// Capacity of control variable state update buffer. A single buffer stores one instance of
  /// control variable state update corresponding to a single call to Controller::qset()
  std::size_t sync_buffer_capacity{ 0 };
};

//=================================================================================================
/// Probe controller runs on the embedded process, and provides mechanisms to
/// - capture periodic snapshots of registered signals (see snap() and flush())
/// - synchronise values of control variables with queued updates (see qset() and sync())
class Controller {
public:
  /// List of operational errors
  enum class Error : std::uint8_t {
    BufferUnavailable,  //!< No free buffer available
    BufferTooSmall,     //!< Buffer is incorrectly sized
    SignalNotFound,     //!< Signal not found
    RoleMismatch,       //!< Expected and actual role of data do not match
    TypeMismatch,       //!< Expected and actual type of data do not match
    SizeMismatch,       //!< Expected and actual size of data do not match
  };

  /// Signature for function to receive log records
  using Receiver = std::function<void(const std::vector<Signal>&, std::span<const std::byte>)>;

  /// Create controller
  /// @param pins Pin configuration
  /// @param buffer_config capture buffer configuration
  /// @param receiver functor to receive records of log and control variable snapshots
  Controller(PinConfig&& pins, const BufferConfig& buffer_config, Receiver&& receiver);

  /// Take a snapshot of current state of watchables and controllables. These values are pushed to
  /// data receiver on subsequent call to flush().
  /// @note This method should be called at the end of a single iteration of realtime process loop
  /// @return void if snapshot was captured successfully; Error::BufferUnavailable if we ran
  /// out of internal buffers. In this case, flush() more frequently to release buffers.
  [[nodiscard]] auto snap() -> std::expected<void, Error>;

  /// Push records captured from previous calls to snap() into the data receiver.
  /// @note This method should only be called from the non-realtime part of the process.
  void flush();

  /// Queue an update for a scalar controllable. This will take effect on next call to sync().
  /// @param name Identyfing name of the controllable as set in configuration
  /// @param value State update to set
  /// @return void on success, otherwise an error code that identifies the failure
  /// @note This method should be called from non-realtime part of the process
  /// @note Method will fail if we run out of sync buffers. In this case, sync() more frequently to
  /// release buffers
  template <NumericType T>
  [[nodiscard]] auto qset(const std::string& name, const T& value) -> std::expected<void, Error>;

  /// Queue an update for a vector controllable. This will take effect on next call to sync().
  /// @param name Identyfing name of the controllable as set in configuration
  /// @param value State update to set
  /// @return void on success, otherwise an error code that identifies the failure
  /// @note This method should be called from non-realtime part of the process
  /// @note Method will fail if we run out of sync buffers. In this case, sync() more frequently to
  /// release buffers
  template <NumericType T>
  [[nodiscard]] auto qset(const std::string& name,
                          std::span<const T> value) -> std::expected<void, Error>;

  /// Queue an update for a controllable with data specified in raw bytes. This will take effect on
  /// next call to sync().
  /// @param name Identyfing name of the controllable as set in configuration
  /// @param value State update to set
  /// @return void on success, otherwise an error code that identifies the failure
  /// @note Method will fail if we run out of sync buffers. In this case, sync() more frequently to
  /// release buffers
  [[nodiscard]] auto qset(const std::string& name,
                          std::span<const std::byte> value) -> std::expected<void, Error>;

  /// Update control variables to values from calls to qset() since the last call to this method
  /// @note This method should be called at the top of realtime process update step
  void sync();

private:
  PinConfig pins_;
  std::vector<Signal>::const_iterator controllables_begin_;
  Receiver receiver_;
  realtime::FIFOBuffer snaps_;
  realtime::FIFOBuffer pending_syncs_;
};

using ControllerException = grape::Exception<Controller::Error>;

//-------------------------------------------------------------------------------------------------
template <NumericType T>
auto PinConfig::pin(const std::string& name, const T& var, Signal::Role role) -> PinConfig& {
  return pin<T>(name, { &var, 1 }, role);
}

//-------------------------------------------------------------------------------------------------
template <NumericType T>
auto PinConfig::pin(const std::string& name, std::span<const T> var,
                    Signal::Role role) -> PinConfig& {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto addr = reinterpret_cast<std::uintptr_t>(var.data());

  signals_.emplace_back(Signal{ .name = name.c_str(),        //
                                .address = addr,             //
                                .num_elements = var.size(),  //
                                .type = toTypeId<T>(),       //
                                .role = role });
  return *this;
}

//-------------------------------------------------------------------------------------------------
template <NumericType T>
auto Controller::qset(const std::string& name, const T& value) -> std::expected<void, Error> {
  return qset<T>(name, { &value, 1 });
}

//-------------------------------------------------------------------------------------------------
template <NumericType T>
auto Controller::qset(const std::string& name,
                      std::span<const T> value) -> std::expected<void, Error> {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto* const data_ptr = reinterpret_cast<const std::byte*>(value.data());
  return qset(name, { data_ptr, value.size_bytes() });
}

}  // namespace grape::probe
