//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/probe/controller.h"

#include <cstring>  // std::memcpy
#include <numeric>  // std::accumulate
#include <thread>

namespace {

using Signal = grape::probe::Signal;

//-------------------------------------------------------------------------------------------------
/// Calculates memory size required to capture a snapshot frame
auto calcSnapFrameSize(const std::vector<Signal>& signals) -> std::size_t {
  return std::accumulate(std::begin(signals), std::end(signals), std::size_t{ 0U },
                         [](const std::uint32_t& a, const Signal& b) {
                           return a + (length(b.type) * b.num_elements);
                         });
}

//-------------------------------------------------------------------------------------------------
/// Calculates memory size required to store a control value update
auto calcSyncFrameSize(const std::vector<Signal>& signals) -> std::size_t {
  // Frame layout: [offset|data (N bytes)] where
  // - offset : offset into signals array
  // - N     : size in bytes of the largest control variable
  std::size_t max_data_size = 0;
  for (const auto& s : signals) {
    if (s.role == Signal::Role::Control) {
      max_data_size = std::max(max_data_size, length(s.type) * s.num_elements);
    }
  }
  using SignalVectorOffset =
      std::iterator_traits<std::vector<Signal>::const_iterator>::difference_type;
  return max_data_size + sizeof(SignalVectorOffset);
}

}  // namespace

namespace grape::probe {

//-------------------------------------------------------------------------------------------------
auto PinConfig::signals() const -> const std::vector<Signal>& {
  return signals_;
}

//-------------------------------------------------------------------------------------------------
auto PinConfig::sort() -> std::vector<Signal>::const_iterator {
  const auto sort_by_role_and_address = [](const Signal& i1, const Signal& i2) -> bool {
    // if roles are the same, compare by address; otherwise, watchables come first
    return ((i1.role == i2.role) ? (i1.address < i2.address) : (i1.role == Signal::Role::Watch));
  };

  // sort for locality and faster access to signals later on
  std::sort(signals_.begin(), signals_.end(), sort_by_role_and_address);

  // return the location of first controllable
  return std::find_if(signals_.begin(), signals_.end(),
                      [](const Signal& s) { return s.role == Signal::Role::Control; });
}

//-------------------------------------------------------------------------------------------------
Controller::Controller(PinConfig&& pins, const BufferConfig& buffer_config, Receiver&& receiver)
  : pins_{ std::move(pins) }
  , controllables_begin_(pins_.sort())
  , receiver_(std::move(receiver))
  , snaps_({ .frame_length = calcSnapFrameSize(pins_.signals()),
             .num_frames = buffer_config.snap_buffer_capacity })
  , pending_syncs_({ .frame_length = calcSyncFrameSize(pins_.signals()),
                     .num_frames = buffer_config.sync_buffer_capacity }) {
}

//-------------------------------------------------------------------------------------------------
auto Controller::snap() -> std::expected<void, Error> {
  auto buffer_check = std::expected<void, Error>{};
  const auto writer = [&signals = pins_.signals(), &buffer_check](std::span<std::byte> buffer) {
    auto offset = 0u;
    const auto buffer_size = buffer.size_bytes();
    for (const auto& s : signals) {
      const auto count = length(s.type) * s.num_elements;
      auto* dest = static_cast<void*>(buffer.data() + offset);
      offset += count;
      if (offset > buffer_size) {
        buffer_check = std::unexpected(Error::BufferTooSmall);
        return;
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
      const auto* src = reinterpret_cast<void*>(s.address);
      std::memcpy(dest, src, count);
    }
  };

  if (not snaps_.visitToWrite(writer)) {
    return std::unexpected(Error::BufferUnavailable);
  }
  return buffer_check;
}

//-------------------------------------------------------------------------------------------------
void Controller::flush() {
  const auto reader = [this](std::span<const std::byte> data) {
    if (this->receiver_ != nullptr) {
      this->receiver_(this->pins_.signals(), data);
    }
  };
  while (snaps_.visitToRead(reader)) {
    // loop until queue is cleared
  }
}

//-------------------------------------------------------------------------------------------------
void Controller::sync() {
  using SignalVectorOffset =
      std::iterator_traits<std::vector<Signal>::const_iterator>::difference_type;

  const auto updater = [&signals = pins_.signals()](std::span<const std::byte> buffer) {
    const auto offset_size = sizeof(SignalVectorOffset);
    SignalVectorOffset offset{};
    std::memcpy(&offset, buffer.data(), offset_size);
    const auto it = signals.begin() + offset;
    const auto count = it->num_elements * length(it->type);
    if (offset_size + count > buffer.size_bytes()) {
      // this should never happen
      panic<ControllerException>("Sync buffer too small", Error::BufferTooSmall);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    std::memcpy(reinterpret_cast<void*>(it->address), buffer.data() + offset_size, count);
  };
  while (pending_syncs_.visitToRead(updater)) {
    // loop until queue is cleared
  }
}

//-------------------------------------------------------------------------------------------------
auto Controller::qset(const std::string& name,
                      std::span<const std::byte> value) -> std::expected<void, Error> {
  const auto& signals = pins_.signals();

  const auto it = std::find_if(controllables_begin_, signals.end(),
                               [&name](const auto& s) { return (name == s.name.cStr()); });

  if (it == signals.end()) {
    return std::unexpected(Error::SignalNotFound);
  }
  if (Signal::Role::Control != it->role) {
    return std::unexpected(Error::RoleMismatch);
  }
  if (value.size_bytes() != it->num_elements * length(it->type)) {
    return std::unexpected(Error::SizeMismatch);
  }

  // Queue the update as [offset|data]
  using SignalVectorOffset =
      std::iterator_traits<std::vector<Signal>::const_iterator>::difference_type;
  const SignalVectorOffset offset = std::distance(signals.begin(), it);
  auto buffer_check = std::expected<void, Error>{};
  const auto writer = [&offset, &value, &buffer_check](std::span<std::byte> buffer) {
    const auto offset_size = sizeof(SignalVectorOffset);
    const auto value_size = value.size_bytes();
    if (offset_size + value_size > buffer.size_bytes()) {
      // this should never happen
      buffer_check = std::unexpected(Error::BufferTooSmall);
      return;
    }
    std::memcpy(buffer.data(), &offset, offset_size);
    std::memcpy(buffer.data() + offset_size, value.data(), value_size);
  };

  if (not pending_syncs_.visitToWrite(writer)) {
    return std::unexpected(Error::BufferUnavailable);
  }
  return buffer_check;
}

}  // namespace grape::probe
