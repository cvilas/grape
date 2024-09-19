//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <cinttypes>
#include <fstream>
#include <optional>
#include <print>
#include <span>
#include <vector>

#include "grape/serdes/serdes.h"

namespace grape::serdes::examples {

//-------------------------------------------------------------------------------------------------
// Example/Test structure
struct State {
  std::string name;
  double timestamp{};
  std::array<double, 3> position{};
};

//-------------------------------------------------------------------------------------------------
// generic file writer
inline auto writeToFile(std::span<const char> data, const std::string& filename) -> bool {
  auto file = std::ofstream(filename, std::ios::binary);
  if (not file) {
    std::println(stderr, "Failed to open '{}' for writing", filename);
    return false;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  file.write(data.data(), static_cast<std::streamsize>(data.size()));
  return true;
}

//-------------------------------------------------------------------------------------------------
// generic file reader
inline auto readFromFile(const std::string& filename) -> std::optional<std::vector<char>> {
  auto file = std::ifstream(filename, std::ios::binary | std::ios::ate);
  if (not file) {
    std::println(stderr, "Failed to open '{}' for reading", filename);
    return {};
  }

  const auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(static_cast<std::size_t>(size));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  file.read(buffer.data(), size);

  return buffer;
}
}  // namespace grape::serdes::examples
