//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#pragma once

#define ZENOHCXX_ZENOHC  // Use the C backend for Zenoh

#include <chrono>
#include <format>
#include <numeric>
#include <print>
#include <zenoh.hxx>

namespace grape::ipc::ex {

//-------------------------------------------------------------------------------------------------
[[nodiscard]] inline auto toString(const zenoh::Id& id) -> std::string {
  return std::accumulate(
      std::begin(id.bytes()), std::end(id.bytes()), std::string(),
      [](const std::string& s, std::uint8_t v) { return std::format("{:02X}", v) + s; });
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(zenoh::SampleKind kind) -> std::string_view {
  switch (kind) {
    case Z_SAMPLE_KIND_PUT:
      return "Put";
    case Z_SAMPLE_KIND_DELETE:
      return "Delete";
  }
  return "";
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] inline auto toNanoSeconds(const zenoh::Timestamp& ts) -> std::uint64_t {
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
[[nodiscard]] inline auto toString(const zenoh::Timestamp& ts) -> std::string {
  const auto ns = std::chrono::nanoseconds(toNanoSeconds(ts));
  const auto tp = std::chrono::system_clock::time_point{
    std::chrono::duration_cast<std::chrono::system_clock::time_point::duration>(ns)
  };
  return std::format("{}", tp);
}

}  // namespace grape::ipc::ex
