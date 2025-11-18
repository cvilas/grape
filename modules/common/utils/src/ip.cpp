//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/utils/ip.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#ifndef HOST_NAME_MAX
#ifdef _POSIX_HOST_NAME_MAX
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#else
#define HOST_NAME_MAX 255  // NOLINT(cppcoreguidelines-macro-usage)
#endif
#endif

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
auto IPAddress::fromString(const std::string& ip_str) -> std::optional<IPAddress> {
  IPAddress addr;
  const auto af_id = ((ip_str.find('.') == std::string::npos) ? AF_INET6 : AF_INET);
  addr.version = ((af_id == AF_INET) ? IPAddress::Version::IPv4 : IPAddress::Version::IPv6);
  const auto ret = inet_pton(af_id, ip_str.c_str(), addr.bytes.data());
  if (ret == 1) {
    return addr;
  }
  return {};
}

//-------------------------------------------------------------------------------------------------
auto toString(const IPAddress& addr) -> std::string {
  auto ip_str = std::array<char, INET6_ADDRSTRLEN>{ 0 };
  const auto af_id = ((addr.version == IPAddress::Version::IPv4) ? AF_INET : AF_INET6);
  if (inet_ntop(af_id, addr.bytes.data(), ip_str.data(), INET6_ADDRSTRLEN) == ip_str.data()) {
    return { ip_str.data() };
  }
  return {};
}

}  // namespace grape::utils
