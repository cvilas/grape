//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <chrono>
#include <exception>
#include <format>
#include <numeric>
#include <zenohc.hxx>

namespace grape::ipc {

//-------------------------------------------------------------------------------------------------
inline auto toString(const zenohc::Id& id) -> std::string {
  return std::accumulate(
      std::begin(id.id), std::end(id.id), std::string(),
      [](const std::string& s, uint8_t v) { return std::format("{:02X}", v) + s; });
}

//-------------------------------------------------------------------------------------------------
constexpr auto toString(const zenohc::WhatAmI& me) -> std::string_view {
  switch (me) {
    case zenohc::WhatAmI::Z_WHATAMI_ROUTER:
      return "Router";
    case zenohc::WhatAmI::Z_WHATAMI_PEER:
      return "Peer";
    case zenohc::WhatAmI::Z_WHATAMI_CLIENT:
      return "Client";
  }
  return "";
}

//-------------------------------------------------------------------------------------------------
constexpr auto toString(zenohc::SampleKind kind) -> std::string_view {
  switch (kind) {
    case Z_SAMPLE_KIND_PUT:
      return "Put";
    case Z_SAMPLE_KIND_DELETE:
      return "Delete";
  }
  return "";
}

//-------------------------------------------------------------------------------------------------
inline auto toString(const zenohc::StrArrayView& arr) -> std::string {
  const auto sz = arr.get_len();
  std::string str = "[";
  for (size_t i = 0; i < sz; i++) {
    str += std::format("\"{:s}\"", arr[i]);
    if (i < sz - 1) {
      str += ", ";
    }
  }
  str += "]";
  return str;
}

//-------------------------------------------------------------------------------------------------
inline auto toNanoSeconds(const zenohc::Timestamp& ts) -> std::uint64_t {
  // NTP64 timestamping: https://docs.rs/zenoh/0.7.2-rc/zenoh/time/struct.NTP64.html
  const auto ntp64 = ts.get_time();
  const auto seconds = static_cast<std::uint32_t>(ntp64 >> 32U);
  const auto fraction = static_cast<std::uint32_t>(ntp64 & 0xFFFFFFFF);

  static constexpr auto NS_PER_S = 1000'000'000;
  const auto sec_to_ns = static_cast<std::uint64_t>(seconds) * NS_PER_S;
  const auto frac_to_ns = static_cast<std::uint64_t>(fraction) * NS_PER_S / (1ULL << 32U);
  return sec_to_ns + frac_to_ns;
}

//-------------------------------------------------------------------------------------------------
inline auto toString(const zenohc::Timestamp& ts) -> std::string {
  const auto ns = std::chrono::nanoseconds(toNanoSeconds(ts));
  const auto tp = std::chrono::system_clock::time_point{
    std::chrono::duration_cast<std::chrono::system_clock::time_point::duration>(ns)
  };
  return std::format("{}", tp);
}

//-------------------------------------------------------------------------------------------------
template <typename T>
constexpr auto expect(std::variant<T, zenohc::ErrorMessage>&& v) -> T {
  if (v.index() == 1) {
    const auto* msg = std::get<zenohc::ErrorMessage>(v).as_string_view().data();
    throw std::runtime_error(std::format("Zenoh Exception: {}", msg));
  }
  return std::get<T>(std::move(v));
}

}  // namespace grape::ipc
