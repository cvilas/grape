//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "font.h"

#include <array>

namespace {
constexpr auto FONT_DATA = std::to_array<unsigned char>({
#embed "./fonts/roboto-mono/RobotoMono-VariableFont_wght.ttf"
});
}  // namespace

namespace grape::plot {

auto fontData() noexcept -> std::span<const unsigned char> {
  return FONT_DATA;
}

}  // namespace grape::plot
