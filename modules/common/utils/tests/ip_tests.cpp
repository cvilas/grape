//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <algorithm>

#include "catch2/catch_test_macros.hpp"
#include "grape/utils/ip.h"

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,bugprone-unchecked-optional-access)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Parses standard address strings to IPAddress", "[IPAddress]") {
  SECTION("Standard IPv4 Address") {
    const auto ip = grape::utils::IPAddress::fromString("192.168.1.1");
    REQUIRE(ip.has_value());
    REQUIRE(ip->version == grape::utils::IPAddress::Version::IPv4);
    const auto expected =
        std::array<std::uint8_t, grape::utils::IPAddress::MAX_SEGMENTS>{ 192, 168, 1, 1 };
    REQUIRE(ip->bytes == expected);
  }

  SECTION("Standard IPv6 address") {
    const auto ip = grape::utils::IPAddress::fromString("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
    REQUIRE(ip.has_value());
    REQUIRE(ip->version == grape::utils::IPAddress::Version::IPv6);
    const auto expected = std::array<std::uint8_t, grape::utils::IPAddress::MAX_SEGMENTS>{
      0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34
    };
    REQUIRE(ip->bytes == expected);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Parses compressed IPv6 address strings to IPAddress", "[IPAddress]") {
  SECTION("Compressed zeros in middle") {
    const auto ip = grape::utils::IPAddress::fromString("2001:db8::1234:5678");
    REQUIRE(ip.has_value());
    REQUIRE(ip->version == grape::utils::IPAddress::Version::IPv6);
    const auto expected = std::array<std::uint8_t, grape::utils::IPAddress::MAX_SEGMENTS>{
      0x20, 0x01, 0x0d, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78
    };
    REQUIRE(ip->bytes == expected);
  }

  SECTION("Compressed leading zeros") {
    const auto ip = grape::utils::IPAddress::fromString("::1");
    REQUIRE(ip.has_value());
    REQUIRE(ip->version == grape::utils::IPAddress::Version::IPv6);
    REQUIRE(std::all_of(ip->bytes.begin(), ip->bytes.end() - 1,
                        [](const auto& v) { return (v == 0); }));
    REQUIRE(ip->bytes[15] == 0x01);
  }

  SECTION("Compressed trailing zeros") {
    const auto ip = grape::utils::IPAddress::fromString("2001:db8::");
    REQUIRE(ip.has_value());
    REQUIRE(ip->version == grape::utils::IPAddress::Version::IPv6);
    REQUIRE(ip->bytes[0] == 0x20);
    REQUIRE(ip->bytes[1] == 0x01);
    REQUIRE(ip->bytes[2] == 0x0d);
    REQUIRE(ip->bytes[3] == 0xb8);
    REQUIRE(std::all_of(ip->bytes.begin() + 4, ip->bytes.end(),
                        [](const auto& v) { return (v == 0); }));
  }

  SECTION("Multiple compressed sections") {
    const auto ip = grape::utils::IPAddress::fromString("2001::1234:5678");
    REQUIRE(ip.has_value());
    REQUIRE(ip->version == grape::utils::IPAddress::Version::IPv6);
    REQUIRE(ip->bytes[0] == 0x20);
    REQUIRE(ip->bytes[1] == 0x01);
    REQUIRE(ip->bytes[12] == 0x12);
    REQUIRE(ip->bytes[13] == 0x34);
    REQUIRE(ip->bytes[14] == 0x56);
    REQUIRE(ip->bytes[15] == 0x78);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Does not parse invalid address strings to IPAddress", "[IPAddress]") {
  SECTION("Invalid IPv4 address formats") {
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("").has_value());
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("256.1.2.3").has_value());
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("1.2.3").has_value());
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("1.2.3.4.5").has_value());
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("a.b.c.d").has_value());
  }

  SECTION("Invalid IPv6 compressed formats") {
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("2001::1234::5678").has_value());
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("2001:db8::gggg").has_value());
    REQUIRE_FALSE(grape::utils::IPAddress::fromString("2001:db8:1:2:3:4:5:6:7").has_value());
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Converts IPv4 address to string", "[IPAddress]") {
  grape::utils::IPAddress ip;
  ip.version = grape::utils::IPAddress::Version::IPv4;
  ip.bytes[0] = 192;
  ip.bytes[1] = 168;
  ip.bytes[2] = 1;
  ip.bytes[3] = 1;
  REQUIRE("192.168.1.1" == grape::utils::toString(ip));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Converts compressed IPv6 address to string", "[IPAddress]") {
  SECTION("Address with zeros in middle") {
    grape::utils::IPAddress ip;
    ip.version = grape::utils::IPAddress::Version::IPv6;
    // Set segments for 2001:db8::1234:5678
    ip.bytes[0] = 0x20;
    ip.bytes[1] = 0x01;
    ip.bytes[2] = 0x0d;
    ip.bytes[3] = 0xb8;
    ip.bytes[12] = 0x12;
    ip.bytes[13] = 0x34;
    ip.bytes[14] = 0x56;
    ip.bytes[15] = 0x78;

    REQUIRE(toString(ip) == "2001:db8::1234:5678");
  }

  SECTION("Address with leading zeros") {
    grape::utils::IPAddress ip;
    ip.version = grape::utils::IPAddress::Version::IPv6;
    // Set last segment to 1 (::1)
    std::fill(ip.bytes.begin(), ip.bytes.begin() + 14, 0);
    ip.bytes[15] = 0x01;

    REQUIRE(toString(ip) == "::1");
  }

  SECTION("Address with trailing zeros") {
    grape::utils::IPAddress ip;
    ip.version = grape::utils::IPAddress::Version::IPv6;
    // Set first segments
    ip.bytes[0] = 0x20;
    ip.bytes[1] = 0x01;
    ip.bytes[2] = 0x0d;
    ip.bytes[3] = 0xb8;
    // Zero out remaining segments
    std::fill(ip.bytes.begin() + 4, ip.bytes.begin() + 16, 0);

    REQUIRE(toString(ip) == "2001:db8::");
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,bugprone-unchecked-optional-access)

}  // namespace
