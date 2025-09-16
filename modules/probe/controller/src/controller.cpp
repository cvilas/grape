//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/probe/controller.h"

#include <algorithm>
#include <cstring>  // std::memcpy
#include <numeric>  // std::accumulate

#include "grape/exception.h"

namespace {

using Signal = grape::probe::Signal;

//-------------------------------------------------------------------------------------------------
/// Calculates memory size required to capture a snapshot frame
auto calcSnapFrameSize(const std::vector<Signal>& signals) -> std::size_t {
  return std::accumulate(std::begin(signals), std::end(signals), std::size_t{ 0U },
                         [](const std::uint32_t& acc, const Signal& sig) -> std::size_t {
                           return acc + (length(sig.type) * sig.num_elements);
                         });
}

//-------------------------------------------------------------------------------------------------
/// Calculates memory size required to store a control value update
auto calcSyncFrameSize(const std::vector<Signal>& signals) -> std::size_t {
  // Frame layout: [offset|data (N bytes)] where
  // - offset : offset into signals array
  // - N     : size in bytes of the largest control variable
  std::size_t max_data_size = 0;
  for (const auto& sig : signals) {
    if (sig.role == Signal::Role::Control) {
      max_data_size = std::max(max_data_size, length(sig.type) * sig.num_elements);
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
auto PinConfig::sort() -> std::ranges::subrange<std::vector<Signal>::const_iterator> {
  const auto compare_by_role_and_address = [](const Signal& i1, const Signal& i2) noexcept -> bool {
    // compare by role, and if roles are the same, then compare by address
    if (i1.role == i2.role) {
      return (i1.address < i2.address);
    }
    return (i1.role < i2.role);
  };

  // sort by role and locality for faster access to signals
  std::ranges::sort(signals_, compare_by_role_and_address);

  // return sub-range of Role::Control
  return std::ranges::equal_range(signals_, Signal::Role::Control, {},
                                  [](const auto& sig) noexcept -> auto { return sig.role; });
}

//-------------------------------------------------------------------------------------------------
Controller::Controller(PinConfig&& pins, const BufferConfig& buffer_config, Receiver&& receiver)
  : pins_{ std::move(pins) }
  , controllables_(pins_.sort())
  , receiver_(std::move(receiver))
  , snaps_({ .frame_length = calcSnapFrameSize(pins_.signals()),
             .num_frames = buffer_config.snap_buffer_capacity })
  , pending_syncs_({ .frame_length = calcSyncFrameSize(pins_.signals()),
                     .num_frames = buffer_config.sync_buffer_capacity }) {
}

//-------------------------------------------------------------------------------------------------
auto Controller::snap() -> Error {
  auto buffer_check = Error::None;
  const auto writer = [&signals = pins_.signals(),
                       &buffer_check](std::span<std::byte> buffer) -> void {
    auto offset = 0UL;
    const auto buffer_size = buffer.size_bytes();
    for (const auto& sig : signals) {
      const auto count = length(sig.type) * sig.num_elements;
      auto subspan = std::span(buffer).subspan(offset);
      auto* dest = static_cast<void*>(subspan.data());
      offset += count;
      if (offset > buffer_size) {
        buffer_check = Error::BufferTooSmall;
        return;
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
      const auto* src = reinterpret_cast<void*>(sig.address);
      std::memcpy(dest, src, count);
    }
  };

  if (not snaps_.visitToWrite(writer)) {
    return Error::BufferUnavailable;
  }
  return buffer_check;
}

//-------------------------------------------------------------------------------------------------
void Controller::flush() {
  const auto reader = [this](std::span<const std::byte> data) -> void {
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

  const auto updater = [&signals = pins_.signals()](std::span<const std::byte> buffer) -> void {
    const auto offset_size = sizeof(SignalVectorOffset);
    SignalVectorOffset offset{};
    std::memcpy(&offset, buffer.data(), offset_size);
    const auto it = signals.begin() + offset;
    const auto count = it->num_elements * length(it->type);
    if (offset_size + count > buffer.size_bytes()) {
      // this should never happen
      panic<Exception>("Sync buffer too small");
    }
    auto subspan = std::span(buffer).subspan(offset_size, count);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::memcpy(std::bit_cast<void*>(it->address), subspan.data(), subspan.size_bytes());
  };
  while (pending_syncs_.visitToRead(updater)) {
    // loop until queue is cleared
  }
}

//-------------------------------------------------------------------------------------------------
auto Controller::qset(const std::string& name, std::span<const std::byte> value) -> Error {
  const auto& signals = pins_.signals();

  const auto it = std::ranges::find_if(controllables_, [&name](const auto& sig) noexcept -> bool {
    return (name == sig.name.cStr());
  });

  if (it == signals.end()) {
    return Error::SignalNotFound;
  }
  if (Signal::Role::Control != it->role) {
    return Error::RoleMismatch;
  }
  if (value.size_bytes() != it->num_elements * length(it->type)) {
    return Error::SizeMismatch;
  }

  // Queue the update as [offset|data]
  using SignalVectorOffset =
      std::iterator_traits<std::vector<Signal>::const_iterator>::difference_type;
  const SignalVectorOffset offset = std::distance(signals.begin(), it);
  auto buffer_check = Error::None;
  const auto writer = [&offset, &value, &buffer_check](std::span<std::byte> buffer) -> void {
    const auto offset_size = sizeof(SignalVectorOffset);
    const auto value_size = value.size_bytes();
    if (offset_size + value_size > buffer.size_bytes()) {
      // this should never happen
      buffer_check = Error::BufferTooSmall;
      return;
    }
    std::memcpy(buffer.data(), &offset, offset_size);
    std::memcpy(&buffer[offset_size], value.data(), value_size);
  };

  if (not pending_syncs_.visitToWrite(writer)) {
    return Error::BufferUnavailable;
  }
  return buffer_check;
}

}  // namespace grape::probe
