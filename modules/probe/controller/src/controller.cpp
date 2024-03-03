//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/probe/controller.h"

#include <cstring>  // std::memcpy
#include <numeric>  // std::accumulate
#include <thread>

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
/// Calculates memory size required to capture a snapshot frame
static auto calcSnapFrameSize(const std::vector<Signal>& signals) -> std::size_t {
  return std::accumulate(
      std::begin(signals), std::end(signals), std::size_t{ 0U },
      [](const std::uint32_t& a, const Signal& b) { return a + b.element_size * b.num_elements; });
}

//-------------------------------------------------------------------------------------------------
/// Calculates memory size required to store a control value update
static auto calcSyncFrameSize(const std::vector<Signal>& signals) -> std::size_t {
  // Frame layout: [offset|data (N bytes)] where
  // - offset : offset into signals array
  // - N     : size in bytes of the largest control variable
  std::size_t max_data_size = 0;
  for (const auto& s : signals) {
    if (s.role == Signal::Role::Control) {
      max_data_size = std::max(max_data_size, s.element_size * s.num_elements);
    }
  }
  using SignalVectorOffset =
      std::iterator_traits<std::vector<Signal>::const_iterator>::difference_type;
  return max_data_size + sizeof(SignalVectorOffset);
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
      const auto count = s.element_size * s.num_elements;
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
    const auto count = it->num_elements * it->element_size;
    if (offset_size + count > buffer.size_bytes()) {
      // this should never happen
      panic<ProbeException>("Sync buffer too small", Error::BufferTooSmall);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
    std::memcpy(reinterpret_cast<void*>(it->address), buffer.data() + offset_size, count);
  };
  while (pending_syncs_.visitToRead(updater)) {
    // loop until queue is cleared
  }
}

}  // namespace grape::probe
