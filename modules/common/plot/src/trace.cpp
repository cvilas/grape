//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/plot/trace.h"

#include <cassert>
#include <cstring>

namespace grape::plot {

//-------------------------------------------------------------------------------------------------
Trace::Trace(std::string name, std::size_t max_history)
  : front_buffer_({ .frame_length = sizeof(Sample), .num_frames = FIFO_CAPACITY })
  , name_(std::move(name))
  , back_buffer_(max_history) {
}

//-------------------------------------------------------------------------------------------------
auto Trace::name() const -> const std::string& {
  return name_;
}

//-------------------------------------------------------------------------------------------------
void Trace::setLineStyle(LineStyle style) {
  line_style_ = style;
}

//-------------------------------------------------------------------------------------------------
auto Trace::lineStyle() const -> LineStyle {
  return line_style_;
}

//-------------------------------------------------------------------------------------------------
void Trace::setPointStyle(PointStyle style) {
  point_style_ = style;
}

//-------------------------------------------------------------------------------------------------
auto Trace::pointStyle() const -> PointStyle {
  return point_style_;
}

//-------------------------------------------------------------------------------------------------
void Trace::setColor(Color col) {
  color_ = col;
}

//-------------------------------------------------------------------------------------------------
auto Trace::color() const -> Color {
  return color_;
}

//-------------------------------------------------------------------------------------------------
void Trace::addData(const Sample& sample) {
  // Enforce monotonically non-decreasing x: binary search and decimation both require sorted x.
  assert(sample.x >= last_x_);
  last_x_ = sample.x;
  // Silently drops if buffer is full (oldest data already in back_buffer_)
  [[maybe_unused]] const auto ok = front_buffer_.visitToWrite(
      [&](std::span<std::byte> frame) { std::memcpy(frame.data(), &sample, sizeof(Sample)); });
}

//-------------------------------------------------------------------------------------------------
auto Trace::snapshot() -> View {
  while (front_buffer_.visitToRead([&](std::span<const std::byte> frame) {
    Sample sample{};
    std::memcpy(&sample, frame.data(), sizeof(Sample));
    back_buffer_.pushBack(sample);
  })) {
  }
  return { .name = name_,
           .samples = back_buffer_.view(),
           .color = color_,
           .line_style = line_style_,
           .point_style = point_style_ };
}

}  // namespace grape::plot
