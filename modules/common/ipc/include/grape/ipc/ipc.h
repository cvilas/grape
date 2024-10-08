//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <format>
#include <numeric>

#define ZENOHCXX_ZENOHC  // Use the C backend for Zenoh
#include <zenoh.hxx>

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
inline auto toString(const zenoh::Id& id) -> std::string {
  return std::accumulate(
      std::begin(id.bytes()), std::end(id.bytes()), std::string(),
      [](const std::string& s, uint8_t v) { return std::format("{:02X}", v) + s; });
}

//-------------------------------------------------------------------------------------------------
constexpr auto toString(const zenoh::WhatAmI& me) -> std::string_view {
  switch (me) {
    case zenoh::WhatAmI::Z_WHATAMI_ROUTER:
      return "Router";
    case zenoh::WhatAmI::Z_WHATAMI_PEER:
      return "Peer";
    case zenoh::WhatAmI::Z_WHATAMI_CLIENT:
      return "Client";
  }
  return "";
}

//-------------------------------------------------------------------------------------------------
constexpr auto toString(zenoh::SampleKind kind) -> std::string_view {
  switch (kind) {
    case Z_SAMPLE_KIND_PUT:
      return "Put";
    case Z_SAMPLE_KIND_DELETE:
      return "Delete";
  }
  return "";
}

//-------------------------------------------------------------------------------------------------
inline auto toNanoSeconds(const zenoh::Timestamp& ts) -> std::uint64_t {
  // NTP64 timestamping: https://docs.rs/zenoh/latest/zenoh/time/struct.NTP64.html
  const auto ntp64 = ts.get_time();
  const auto seconds = static_cast<std::uint32_t>(ntp64 >> 32U);
  const auto fraction = static_cast<std::uint32_t>(ntp64 & 0xFFFFFFFF);

  static constexpr auto NS_PER_S = 1000'000'000;
  const auto sec_to_ns = static_cast<std::uint64_t>(seconds) * NS_PER_S;
  const auto frac_to_ns = static_cast<std::uint64_t>(fraction) * NS_PER_S / (1ULL << 32U);
  return sec_to_ns + frac_to_ns;
}

//-------------------------------------------------------------------------------------------------
inline auto toString(const zenoh::Timestamp& ts) -> std::string {
  const auto ns = std::chrono::nanoseconds(toNanoSeconds(ts));
  const auto tp = std::chrono::system_clock::time_point{
    std::chrono::duration_cast<std::chrono::system_clock::time_point::duration>(ns)
  };
  return std::format("{}", tp);
}

}  // namespace grape::ipc
