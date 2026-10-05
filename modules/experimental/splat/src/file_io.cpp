//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/splat/file_io.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <format>
#include <fstream>
#include <memory>
#include <optional>
#include <string_view>
#include <system_error>
#include <utility>

namespace {

using grape::splat::Quat;
using grape::splat::Splat;

static_assert(std::endian::native == std::endian::little,
              "File decoders assume a little-endian host");

/// Zeroth-order spherical harmonics basis constant: 1 / (2·sqrt(π))
constexpr auto SH_C0 = 0.28209479177387814F;
constexpr auto COLOR_MAX = 255.F;
constexpr auto QUAT_SCALE = 128.F;

//-------------------------------------------------------------------------------------------------
template <typename T>
auto readAs(std::span<const std::byte> data, std::size_t offset) -> T {
  auto value = T{};
  std::memcpy(&value, data.subspan(offset, sizeof(T)).data(), sizeof(T));
  return value;
}

//-------------------------------------------------------------------------------------------------
template <typename T>
void writeAs(std::span<std::byte> data, std::size_t offset, const T& value) {
  std::memcpy(data.subspan(offset, sizeof(T)).data(), &value, sizeof(T));
}

//-------------------------------------------------------------------------------------------------
auto toUnorm8(float value) -> std::uint8_t {
  return static_cast<std::uint8_t>(std::clamp(std::round(value * COLOR_MAX), 0.F, COLOR_MAX));
}

//-------------------------------------------------------------------------------------------------
auto sigmoid(float value) -> float {
  return 1.F / (1.F + std::exp(-value));
}

//=================================================================================================
// .splat format
//=================================================================================================

constexpr auto SPLAT_POSITION_OFFSET = 0U;
constexpr auto SPLAT_SCALE_OFFSET = 12U;
constexpr auto SPLAT_COLOR_OFFSET = 24U;
constexpr auto SPLAT_ROTATION_OFFSET = 28U;

//-------------------------------------------------------------------------------------------------
auto decodeQuantisedQuat(std::array<std::uint8_t, 4> bytes) -> Quat {
  const auto unpack = [](std::uint8_t byte) {
    return (static_cast<float>(byte) - QUAT_SCALE) / QUAT_SCALE;
  };
  return normalize(Quat{
      .w = unpack(bytes.at(0)),
      .x = unpack(bytes.at(1)),
      .y = unpack(bytes.at(2)),
      .z = unpack(bytes.at(3)),
  });
}

//-------------------------------------------------------------------------------------------------
auto encodeQuantisedQuat(const Quat& quat) -> std::array<std::uint8_t, 4> {
  const auto pack = [](float value) {
    return static_cast<std::uint8_t>(
        std::clamp(std::round((value * QUAT_SCALE) + QUAT_SCALE), 0.F, COLOR_MAX));
  };
  const auto qn = normalize(quat);
  return { pack(qn.w), pack(qn.x), pack(qn.y), pack(qn.z) };
}

//=================================================================================================
// PLY format
//=================================================================================================

//-------------------------------------------------------------------------------------------------
enum class PlyType : std::uint8_t { Int8, UInt8, Int16, UInt16, Int32, UInt32, Float32, Float64 };

//-------------------------------------------------------------------------------------------------
auto parsePlyType(std::string_view name) -> std::optional<PlyType> {
  static constexpr auto TYPES = std::to_array<std::pair<std::string_view, PlyType>>({
      { "char", PlyType::Int8 },
      { "int8", PlyType::Int8 },
      { "uchar", PlyType::UInt8 },
      { "uint8", PlyType::UInt8 },
      { "short", PlyType::Int16 },
      { "int16", PlyType::Int16 },
      { "ushort", PlyType::UInt16 },
      { "uint16", PlyType::UInt16 },
      { "int", PlyType::Int32 },
      { "int32", PlyType::Int32 },
      { "uint", PlyType::UInt32 },
      { "uint32", PlyType::UInt32 },
      { "float", PlyType::Float32 },
      { "float32", PlyType::Float32 },
      { "double", PlyType::Float64 },
      { "float64", PlyType::Float64 },
  });
  const auto* const it =
      std::ranges::find(TYPES, name, &std::pair<std::string_view, PlyType>::first);
  if (it == TYPES.end()) {
    return std::nullopt;
  }
  return it->second;
}

//-------------------------------------------------------------------------------------------------
constexpr auto sizeOf(PlyType type) -> std::size_t {
  switch (type) {
    case PlyType::Int8:
      return sizeof(std::int8_t);
    case PlyType::UInt8:
      return sizeof(std::uint8_t);
    case PlyType::Int16:
      return sizeof(std::int16_t);
    case PlyType::UInt16:
      return sizeof(std::uint16_t);
    case PlyType::Int32:
      return sizeof(std::int32_t);
    case PlyType::UInt32:
      return sizeof(std::uint32_t);
    case PlyType::Float32:
      return sizeof(float);
    case PlyType::Float64:
      return sizeof(double);
  }
  return 0;
}

//-------------------------------------------------------------------------------------------------
auto readPlyScalar(std::span<const std::byte> data, std::size_t offset, PlyType type) -> float {
  switch (type) {
    case PlyType::Int8:
      return static_cast<float>(readAs<std::int8_t>(data, offset));
    case PlyType::UInt8:
      return static_cast<float>(readAs<std::uint8_t>(data, offset));
    case PlyType::Int16:
      return static_cast<float>(readAs<std::int16_t>(data, offset));
    case PlyType::UInt16:
      return static_cast<float>(readAs<std::uint16_t>(data, offset));
    case PlyType::Int32:
      return static_cast<float>(readAs<std::int32_t>(data, offset));
    case PlyType::UInt32:
      return static_cast<float>(readAs<std::uint32_t>(data, offset));
    case PlyType::Float32:
      return readAs<float>(data, offset);
    case PlyType::Float64:
      return static_cast<float>(readAs<double>(data, offset));
  }
  return 0.F;
}

//-------------------------------------------------------------------------------------------------
struct PlyProperty {
  std::string name;
  PlyType type{ PlyType::Float32 };
  std::size_t offset{ 0 };  //!< byte offset within the element record
};

//-------------------------------------------------------------------------------------------------
struct PlyElement {
  std::string name;
  std::size_t count{ 0 };
  std::size_t stride{ 0 };          //!< bytes per record
  bool has_list_property{ false };  //!< records are variable size
  std::vector<PlyProperty> properties;
};

//-------------------------------------------------------------------------------------------------
struct PlyHeader {
  std::size_t data_offset{ 0 };  //!< byte offset of the first element record
  std::vector<PlyElement> elements;
};

//-------------------------------------------------------------------------------------------------
/// Split a line into whitespace separated tokens
auto tokenize(std::string_view line) -> std::vector<std::string_view> {
  auto tokens = std::vector<std::string_view>{};
  auto pos = std::size_t{ 0 };
  while (pos < line.size()) {
    const auto start = line.find_first_not_of(" \t", pos);
    if (start == std::string_view::npos) {
      break;
    }
    const auto end = std::min(line.find_first_of(" \t", start), line.size());
    tokens.push_back(line.substr(start, end - start));
    pos = end;
  }
  return tokens;
}

//-------------------------------------------------------------------------------------------------
auto parsePlyElement(std::span<const std::string_view> tokens, unsigned line_number)
    -> std::expected<PlyElement, std::string> {
  if (tokens.size() != 3) {
    return std::unexpected(std::format("PLY: Malformed element at line {}", line_number));
  }
  auto count = std::size_t{ 0 };
  const auto count_str = tokens.at(2);
  const auto* const count_begin = std::to_address(count_str.begin());
  const auto* const count_end = std::to_address(count_str.end());
  const auto [ptr, ec] = std::from_chars(count_begin, count_end, count);
  if (ec != std::errc{} or ptr != count_end) {
    return std::unexpected(std::format("PLY: Invalid element count at line {}", line_number));
  }
  return PlyElement{
    .name = std::string(tokens.at(1)),
    .count = count,
    .stride = 0,
    .has_list_property = false,
    .properties = {},
  };
}

//-------------------------------------------------------------------------------------------------
auto parsePlyProperty(std::span<const std::string_view> tokens, unsigned line_number,
                      PlyElement& element) -> std::expected<void, std::string> {
  if (tokens.size() >= 2 and tokens.at(1) == "list") {
    element.has_list_property = true;
    return {};
  }
  if (tokens.size() != 3) {
    return std::unexpected(std::format("PLY: Malformed property at line {}", line_number));
  }
  const auto type = parsePlyType(tokens.at(1));
  if (not type) {
    return std::unexpected(
        std::format("PLY: Unknown type '{}' at line {}", tokens.at(1), line_number));
  }
  element.properties.push_back(
      { .name = std::string(tokens.at(2)), .type = *type, .offset = element.stride });
  element.stride += sizeOf(*type);
  return {};
}

//-------------------------------------------------------------------------------------------------
auto parsePlyHeader(std::span<const std::byte> data) -> std::expected<PlyHeader, std::string> {
  static constexpr auto END_HEADER = std::string_view{ "end_header" };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto text = std::string_view{ reinterpret_cast<const char*>(data.data()), data.size() };

  auto header = PlyHeader{};
  auto pos = std::size_t{ 0 };
  auto line_number = 0U;
  auto has_format = false;
  while (true) {
    const auto eol = text.find('\n', pos);
    if (eol == std::string_view::npos) {
      return std::unexpected("PLY: Missing 'end_header'");
    }
    auto line = text.substr(pos, eol - pos);
    pos = eol + 1;
    ++line_number;
    if (line.ends_with('\r')) {
      line.remove_suffix(1);
    }

    const auto tokens = tokenize(line);
    if (line_number == 1) {
      if (tokens.size() != 1 or tokens.at(0) != "ply") {
        return std::unexpected("PLY: Missing 'ply' signature");
      }
      continue;
    }
    if (tokens.empty() or tokens.at(0) == "comment" or tokens.at(0) == "obj_info") {
      continue;
    }
    if (tokens.at(0) == END_HEADER) {
      break;
    }
    if (tokens.at(0) == "format") {
      if (tokens.size() < 2 or tokens.at(1) != "binary_little_endian") {
        return std::unexpected("PLY: Only 'binary_little_endian' format is supported");
      }
      has_format = true;
    } else if (tokens.at(0) == "element") {
      auto element = parsePlyElement(tokens, line_number);
      if (not element) {
        return std::unexpected(element.error());
      }
      header.elements.push_back(std::move(*element));
    } else if (tokens.at(0) == "property") {
      if (header.elements.empty()) {
        return std::unexpected(std::format("PLY: Property before element at line {}", line_number));
      }
      const auto result = parsePlyProperty(tokens, line_number, header.elements.back());
      if (not result) {
        return std::unexpected(result.error());
      }
    } else {
      return std::unexpected(
          std::format("PLY: Unexpected keyword '{}' at line {}", tokens.at(0), line_number));
    }
  }
  if (not has_format) {
    return std::unexpected("PLY: Missing 'format'");
  }
  header.data_offset = pos;
  return header;
}

}  // namespace

namespace grape::splat {

//-------------------------------------------------------------------------------------------------
auto decodeSplat(std::span<const std::byte> data)
    -> std::expected<std::vector<Splat>, std::string> {
  if (data.size() % SPLAT_RECORD_SIZE != 0) {
    return std::unexpected(std::format("SPLAT: Size ({} bytes) is not a multiple of {}",
                                       data.size(), SPLAT_RECORD_SIZE));
  }
  const auto count = data.size() / SPLAT_RECORD_SIZE;
  auto splats = std::vector<Splat>{};
  splats.reserve(count);
  for (auto i = 0UZ; i < count; ++i) {
    const auto record = data.subspan(i * SPLAT_RECORD_SIZE, SPLAT_RECORD_SIZE);
    const auto pos = readAs<std::array<float, 3>>(record, SPLAT_POSITION_OFFSET);
    const auto scale = readAs<std::array<float, 3>>(record, SPLAT_SCALE_OFFSET);
    const auto color = readAs<std::array<std::uint8_t, 4>>(record, SPLAT_COLOR_OFFSET);
    const auto rot = readAs<std::array<std::uint8_t, 4>>(record, SPLAT_ROTATION_OFFSET);
    splats.push_back({
        .position = { .x = pos.at(0), .y = pos.at(1), .z = pos.at(2) },
        .scale = { .x = scale.at(0), .y = scale.at(1), .z = scale.at(2) },
        .rotation = decodeQuantisedQuat(rot),
        .color = { .r = color.at(0), .g = color.at(1), .b = color.at(2), .a = color.at(3) },
    });
  }
  return splats;
}

//-------------------------------------------------------------------------------------------------
auto encodeSplat(std::span<const Splat> splats) -> std::vector<std::byte> {
  auto data = std::vector<std::byte>(splats.size() * SPLAT_RECORD_SIZE);
  const auto out = std::span{ data };
  for (auto i = 0UZ; i < splats.size(); ++i) {
    const auto& splat = splats.at(i);
    const auto record = out.subspan(i * SPLAT_RECORD_SIZE, SPLAT_RECORD_SIZE);
    writeAs(record, SPLAT_POSITION_OFFSET,
            std::array{ splat.position.x, splat.position.y, splat.position.z });
    writeAs(record, SPLAT_SCALE_OFFSET, std::array{ splat.scale.x, splat.scale.y, splat.scale.z });
    writeAs(record, SPLAT_COLOR_OFFSET,
            std::array{ splat.color.r, splat.color.g, splat.color.b, splat.color.a });
    writeAs(record, SPLAT_ROTATION_OFFSET, encodeQuantisedQuat(splat.rotation));
  }
  return data;
}

//-------------------------------------------------------------------------------------------------
auto decodePly(std::span<const std::byte> data) -> std::expected<std::vector<Splat>, std::string> {
  const auto header = parsePlyHeader(data);
  if (not header) {
    return std::unexpected(header.error());
  }

  // Locate the vertex element. Elements preceding it must have fixed size records.
  auto offset = header->data_offset;
  const PlyElement* vertex = nullptr;
  for (const auto& element : header->elements) {
    if (element.name == "vertex") {
      vertex = &element;
      break;
    }
    if (element.has_list_property) {
      return std::unexpected(std::format(
          "PLY: Unsupported list property in element '{}' before 'vertex'", element.name));
    }
    offset += element.count * element.stride;
  }
  if (vertex == nullptr) {
    return std::unexpected("PLY: Missing 'vertex' element");
  }
  if (vertex->has_list_property) {
    return std::unexpected("PLY: List properties in 'vertex' are not supported");
  }
  if (offset + (vertex->count * vertex->stride) > data.size()) {
    return std::unexpected("PLY: File is truncated");
  }

  // Map required attributes to properties
  static constexpr auto ATTRIBUTES = std::array<std::string_view, 14>{
    "x",       "y",       "z",       "f_dc_0", "f_dc_1", "f_dc_2", "opacity",
    "scale_0", "scale_1", "scale_2", "rot_0",  "rot_1",  "rot_2",  "rot_3",
  };
  auto props = std::array<const PlyProperty*, ATTRIBUTES.size()>{};
  for (auto i = 0UZ; i < ATTRIBUTES.size(); ++i) {
    const auto it = std::ranges::find(vertex->properties, ATTRIBUTES.at(i), &PlyProperty::name);
    if (it == vertex->properties.end()) {
      return std::unexpected(std::format("PLY: Missing vertex property '{}'", ATTRIBUTES.at(i)));
    }
    props.at(i) = &(*it);
  }

  auto splats = std::vector<Splat>{};
  splats.reserve(vertex->count);
  auto values = std::array<float, ATTRIBUTES.size()>{};
  for (auto idx = 0UZ; idx < vertex->count; ++idx) {
    const auto record = offset + (idx * vertex->stride);
    for (auto i = 0UZ; i < props.size(); ++i) {
      values.at(i) = readPlyScalar(data, record + props.at(i)->offset, props.at(i)->type);
    }
    // Training optimises unconstrained numbers. The PLY stores those raw values, and each one is
    // mapped into its valid range here, exactly as the training code does when rendering:
    // - scale: stored as log(σ), so σ = exp(value) is always positive
    // - opacity: stored as a logit, so α = sigmoid(value) is always in (0, 1)
    // - rotation: any non-zero quaternion, normalised to a unit quaternion
    // - colour: zeroth-order spherical harmonic coefficient. The view-independent colour is
    //   SH_C0·f_dc, offset by 0.5 so that a zero coefficient is mid-grey.
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    splats.push_back({
        .position = { .x = values[0], .y = values[1], .z = values[2] },
        .scale = { .x = std::exp(values[7]), .y = std::exp(values[8]), .z = std::exp(values[9]) },
        .rotation =
            normalize(Quat{ .w = values[10], .x = values[11], .y = values[12], .z = values[13] }),
        .color = { .r = toUnorm8(0.5F + (SH_C0 * values[3])),
                   .g = toUnorm8(0.5F + (SH_C0 * values[4])),
                   .b = toUnorm8(0.5F + (SH_C0 * values[5])),
                   .a = toUnorm8(sigmoid(values[6])),
        },
    });
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
  }
  return splats;
}

//-------------------------------------------------------------------------------------------------
auto load(const std::filesystem::path& path) -> std::expected<std::vector<Splat>, std::string> {
  auto ext = path.extension().string();
  std::ranges::transform(ext, ext.begin(), [](char ch) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  });
  if (ext != ".splat" and ext != ".ply") {
    return std::unexpected(std::format("Unsupported file extension '{}'", ext));
  }

  auto file = std::ifstream(path, std::ios::binary | std::ios::ate);
  if (not file) {
    return std::unexpected(std::format("Unable to open '{}'", path.string()));
  }
  const auto size = static_cast<std::size_t>(file.tellg());
  auto data = std::vector<std::byte>(size);
  file.seekg(0);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (not file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size))) {
    return std::unexpected(std::format("Unable to read '{}'", path.string()));
  }
  return (ext == ".splat") ? decodeSplat(data) : decodePly(data);
}

//-------------------------------------------------------------------------------------------------
auto save(const std::filesystem::path& path, std::span<const Splat> splats)
    -> std::expected<void, std::string> {
  const auto data = encodeSplat(splats);
  auto file = std::ofstream(path, std::ios::binary | std::ios::trunc);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (not file or not file.write(reinterpret_cast<const char*>(data.data()),
                                 static_cast<std::streamsize>(data.size()))) {
    return std::unexpected(std::format("Unable to write '{}'", path.string()));
  }
  return {};
}

}  // namespace grape::splat
