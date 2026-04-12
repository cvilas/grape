//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/plot/trace.h"

#include <cstring>

namespace grape::plot {

//-------------------------------------------------------------------------------------------------
Trace::Trace(std::string name, std::size_t max_history)
  : buf_({ .frame_length = sizeof(Sample), .num_frames = FIFO_CAPACITY })
  , capacity_(max_history)
  , name_(std::move(name)) {
  snap_.reserve(capacity_);
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
  // Silently drops if buffer is full (oldest data already in snap_)
  [[maybe_unused]] const auto ok = buf_.visitToWrite(
      [&](std::span<std::byte> frame) { std::memcpy(frame.data(), &sample, sizeof(Sample)); });
}

//-------------------------------------------------------------------------------------------------
auto Trace::snapshot() -> View {
  while (buf_.visitToRead([&](std::span<const std::byte> frame) {
    Sample sample{};
    std::memcpy(&sample, frame.data(), sizeof(Sample));
    snap_.push_back(sample);
  })) {
  }
  if (snap_.size() > capacity_) {
    snap_.erase(snap_.begin(),
                snap_.begin() + static_cast<std::ptrdiff_t>(snap_.size() - capacity_));
  }
  return { .name = name_,
           .samples = snap_,
           .color = color_,
           .line_style = line_style_,
           .point_style = point_style_ };
}

}  // namespace grape::plot
