//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "grape/probe/monitor.h"

#include <cstdio>
#include <limits>
#include <mutex>
#include <print>
#include <shared_mutex>

#include <SDL3/SDL.h>

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "grape/exception.h"
#include "imgui.h"
#include "implot.h"

namespace {

//=================================================================================================
// Buffers signal data frames
class ScrollingBuffer {
public:
  /// identifies a signal and a specific trace in that signal (if the signal is multivariate)
  struct TraceID {
    std::size_t index{};      //!< signal index
    std::size_t sub_index{};  //!< index of trace within signal
  };

  /// Construct buffer
  /// @param length Number of snapshot frames to hold in the buffer
  /// @param signals_info Layout of signals in a snapshot frame
  ScrollingBuffer(std::size_t length, const std::vector<grape::probe::Signal>& signals_info);

  /// Append snapshot frame to buffer
  void addFrame(std::span<const std::byte> frame);

  /// Convenience function. Mark a specific sub-signal within a signal
  void markTrace(TraceID id);

  /// @return The last trace marked
  [[nodiscard]] auto markedTrace() const -> TraceID;

  /// @return Information about layout of signals in a snapshot frame
  [[nodiscard]] auto signalsInfo() const -> const std::vector<grape::probe::Signal>&;

  /// @return Number of snapshot frames the buffer can hold
  [[nodiscard]] auto length() const -> std::size_t;

  /// Returns pointer to raw frame data for a specified index
  /// @param idx Index in range [0, length()]
  /// @return Pointer to raw frame data
  [[nodiscard]] auto frameData(std::size_t idx) const -> std::byte const*;

  /// @return Array containing locations of signal data within a single snapshot frame
  [[nodiscard]] auto signalDataOffsetsWithinFrame() const -> const std::vector<std::size_t>&;

  /// @return Location of timestamp data within a single snapshot frame
  [[nodiscard]] auto timestampDataOffsetWithinFrame() const -> std::size_t;

  /// @return Timestamp data type
  [[nodiscard]] auto timestampDataType() const -> grape::probe::TypeId;

private:
  std::size_t length_{};
  std::size_t frame_size_{};
  std::size_t head_index_{};  //!< next write location in range [0, length]
  TraceID trace_id_{};
  std::vector<grape::probe::Signal> signals_info_;
  grape::probe::TypeId timestamp_type_{ grape::probe::TypeId::Float64 };
  std::size_t timestamp_offset_in_frame_{ std::numeric_limits<std::size_t>::max() };
  std::vector<std::size_t> signal_offsets_in_frame_;
  std::vector<std::byte> frame_data_;
};

//-------------------------------------------------------------------------------------------------
ScrollingBuffer::ScrollingBuffer(std::size_t length,
                                 const std::vector<grape::probe::Signal>& signals_info)
  : length_(length), signals_info_(signals_info) {
  const auto num_signals = signals_info_.size();
  signal_offsets_in_frame_.resize(num_signals);
  for (auto signal_number = 0U; signal_number < num_signals; ++signal_number) {
    const auto& signal_info = signals_info_.at(signal_number);
    if (signal_info.role == grape::probe::Signal::Role::Timestamp) {
      timestamp_offset_in_frame_ = frame_size_;
    }
    signal_offsets_in_frame_[signal_number] = frame_size_;
    frame_size_ += grape::probe::length(signal_info.type) * signal_info.num_elements;
  }
  if (timestamp_offset_in_frame_ == std::numeric_limits<std::size_t>::max()) {
    grape::panic<grape::Exception>(
        std::format("Timestamp {}", toString(grape::probe::Monitor::Error::SignalNotFound)));
  }
  frame_data_.resize(length_ * frame_size_);
}

//-------------------------------------------------------------------------------------------------
void ScrollingBuffer::addFrame(std::span<const std::byte> frame) {
  const auto passed_frame_size = frame.size_bytes();
  if (passed_frame_size != frame_size_) {
    grape::panic<grape::Exception>(
        std::format("Expected frame size: {} bytes, got {} bytes", frame_size_, passed_frame_size));
  }
  using OffsetType = std::vector<std::byte>::difference_type;
  const auto offset = static_cast<OffsetType>(head_index_ * frame_size_);
  std::ranges::copy(frame, frame_data_.begin() + offset);
  head_index_ = (head_index_ + 1) % length_;
}

//-------------------------------------------------------------------------------------------------
void ScrollingBuffer::markTrace(TraceID id) {
  trace_id_ = id;
}

//-------------------------------------------------------------------------------------------------
auto ScrollingBuffer::markedTrace() const -> TraceID {
  return trace_id_;
}

//-------------------------------------------------------------------------------------------------
auto ScrollingBuffer::signalsInfo() const -> const std::vector<grape::probe::Signal>& {
  return signals_info_;
}

//-------------------------------------------------------------------------------------------------
auto ScrollingBuffer::length() const -> std::size_t {
  return length_;
}

//-------------------------------------------------------------------------------------------------
auto ScrollingBuffer::frameData(std::size_t idx) const -> std::byte const* {
  const auto frame_number = (head_index_ + idx) % length_;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  return frame_data_.data() + (frame_number * frame_size_);
}

//-------------------------------------------------------------------------------------------------
auto ScrollingBuffer::signalDataOffsetsWithinFrame() const -> const std::vector<std::size_t>& {
  return signal_offsets_in_frame_;
}

//-------------------------------------------------------------------------------------------------
auto ScrollingBuffer::timestampDataOffsetWithinFrame() const -> std::size_t {
  return timestamp_offset_in_frame_;
}

//-------------------------------------------------------------------------------------------------
auto ScrollingBuffer::timestampDataType() const -> grape::probe::TypeId {
  return timestamp_type_;
}

//-------------------------------------------------------------------------------------------------
auto convert(grape::probe::TypeId tid, const std::byte* bytes) -> double {
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  using TID = grape::probe::TypeId;
  switch (tid) {
    case TID::Int8:
      return static_cast<double>(*reinterpret_cast<const std::int8_t*>(bytes));
    case TID::Uint8:
      return static_cast<double>(*reinterpret_cast<const std::uint8_t*>(bytes));
    case TID::Int16:
      return static_cast<double>(*reinterpret_cast<const std::int16_t*>(bytes));
    case TID::Uint16:
      return static_cast<double>(*reinterpret_cast<const std::uint16_t*>(bytes));
    case TID::Int32:
      return static_cast<double>(*reinterpret_cast<const std::int32_t*>(bytes));
    case TID::Uint32:
      return static_cast<double>(*reinterpret_cast<const std::uint32_t*>(bytes));
    case TID::Int64:
      return static_cast<double>(*reinterpret_cast<const std::int64_t*>(bytes));
    case TID::Uint64:
      return static_cast<double>(*reinterpret_cast<const std::uint64_t*>(bytes));
    case TID::Float32:
      return static_cast<double>(*reinterpret_cast<const float*>(bytes));
    case TID::Float64:
      return static_cast<double>(*reinterpret_cast<const double*>(bytes));
  }
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
  std::unreachable();
}

//-------------------------------------------------------------------------------------------------
auto toImGuiDataType(grape::probe::TypeId id) -> ImGuiDataType {
  switch (id) {
      // clang-format off
    case grape::probe::TypeId::Int8: return ImGuiDataType_S8;
    case grape::probe::TypeId::Uint8: return ImGuiDataType_U8;
    case grape::probe::TypeId::Int16: return ImGuiDataType_S16;
    case grape::probe::TypeId::Uint16: return ImGuiDataType_U16;
    case grape::probe::TypeId::Int32: return ImGuiDataType_S32;
    case grape::probe::TypeId::Uint32: return ImGuiDataType_U32;
    case grape::probe::TypeId::Int64: return ImGuiDataType_S64;
    case grape::probe::TypeId::Uint64: return ImGuiDataType_U64;
    case grape::probe::TypeId::Float32: return ImGuiDataType_Float;
    case grape::probe::TypeId::Float64: return ImGuiDataType_Double;
      // clang-format on
  }
  std::unreachable();
}

//-------------------------------------------------------------------------------------------------
auto signalDataGetter(int idx, void* buf) -> ImPlotPoint {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* const buffer = reinterpret_cast<ScrollingBuffer*>(buf);
  if (buffer == nullptr) {
    return {};
  }

  // TODO(vilas): pass frame_data, trace, signal_info from outside to optimise number of times they
  // are calculated

  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  const auto* frame_data = buffer->frameData(static_cast<std::size_t>(idx));
  const auto trace = buffer->markedTrace();
  const auto& signal_info = buffer->signalsInfo().at(trace.index);
  const auto* signal_data = frame_data + buffer->signalDataOffsetsWithinFrame().at(trace.index);
  const auto* trace_data = signal_data + (trace.sub_index * grape::probe::length(signal_info.type));
  const auto* timestamp_data = frame_data + buffer->timestampDataOffsetWithinFrame();
  const auto timestamp_type = buffer->timestampDataType();
  // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  ImPlotPoint pt;
  pt.x = convert(timestamp_type, timestamp_data);
  pt.y = convert(signal_info.type, trace_data);
  return pt;
}

//=================================================================================================
// Storage for control variable updates
class Controllables {
public:
  struct Item {
    grape::probe::Signal info;
    std::vector<std::byte> data;
  };

  Controllables(const std::vector<grape::probe::Signal>& signals_info,
                std::span<const std::byte> frame);
  auto items() -> std::vector<Item>&;

private:
  std::vector<Item> items_;
};

//-------------------------------------------------------------------------------------------------
Controllables::Controllables(const std::vector<grape::probe::Signal>& signals_info,
                             std::span<const std::byte> frame) {
  items_.reserve(signals_info.size());
  auto offset = 0UL;
  for (const auto& sig : signals_info) {
    const auto count = length(sig.type) * sig.num_elements;
    if (sig.role == grape::probe::Signal::Role::Control) {
      const auto data = frame.subspan(offset, count);
      items_.emplace_back(Item{ .info = sig, .data = { data.begin(), data.end() } });
    }
    offset += count;
  }
}

//-------------------------------------------------------------------------------------------------
auto Controllables::items() -> std::vector<Item>& {
  return items_;
}

}  // namespace

namespace grape::probe {

struct Monitor::Impl {
  SDL_Window* window{ nullptr };
  SDL_Renderer* renderer{ nullptr };
  ImGuiContext* imgui_ctx{ nullptr };
  ImPlotContext* implot_ctx{ nullptr };
  Monitor::Sender sender{ nullptr };
  std::shared_mutex signals_lock;
  std::unique_ptr<ScrollingBuffer> signals_buffer;
  std::shared_mutex controllables_lock;
  std::unique_ptr<Controllables> controllables;
};

//-------------------------------------------------------------------------------------------------
Monitor::Monitor() : impl_{ std::make_unique<Impl>() } {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    panic<Exception>(std::format("SDL_Init: {}", SDL_GetError()));
  }

  // Create window with graphics context
  static constexpr auto WIN_W = 1280;
  static constexpr auto WIN_H = 720;
  const auto win_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  impl_->window = SDL_CreateWindow("Implot Example", WIN_W, WIN_H, win_flags);
  if (impl_->window == nullptr) {
    panic<Exception>(std::format("SDL_CreateWindow: {}", SDL_GetError()));
  }
  impl_->renderer = SDL_CreateRenderer(impl_->window, nullptr);
  SDL_SetRenderVSync(impl_->renderer, 1);
  if (impl_->renderer == nullptr) {
    panic<Exception>(std::format("Error: SDL_CreateRenderer(): {}", SDL_GetError()));
  }
  SDL_ShowWindow(impl_->window);

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  impl_->imgui_ctx = ImGui::CreateContext();
  if (nullptr == impl_->imgui_ctx) {
    panic<Exception>(std::format("ImGui::CreateContext: {}", toString(Error::Renderer)));
  }
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // NOLINT(hicpp-signed-bitwise)

  ImGui::StyleColorsDark();

  if (not ImGui_ImplSDL3_InitForSDLRenderer(impl_->window, impl_->renderer)) {
    panic<Exception>(std::format("Error: ImGui_ImplSDL3_InitForSDLRenderer: {}", SDL_GetError()));
  }

  if (not ImGui_ImplSDLRenderer3_Init(impl_->renderer)) {
    panic<Exception>(std::format("Error: ImGui_ImplSDLRenderer3_Init: {}", SDL_GetError()));
  }

  impl_->implot_ctx = ImPlot::CreateContext();
  if (nullptr == impl_->implot_ctx) {
    std::ignore = std::fputs("Error: ImPlot::CreateContext\n", stderr);
  }
}

//-------------------------------------------------------------------------------------------------
Monitor::~Monitor() {
  if (impl_->implot_ctx != nullptr) {
    ImPlot::DestroyContext(impl_->implot_ctx);
  }
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  if (impl_->imgui_ctx != nullptr) {
    ImGui::DestroyContext(impl_->imgui_ctx);
  }
  SDL_DestroyRenderer(impl_->renderer);
  SDL_DestroyWindow(impl_->window);
  SDL_Quit();
}

//-------------------------------------------------------------------------------------------------
void Monitor::setSender(Sender&& sender) {
  const std::unique_lock ctrl_lock(impl_->controllables_lock);
  impl_->sender = std::move(sender);
}

//-------------------------------------------------------------------------------------------------
void Monitor::run() {
  static constexpr auto BK_COLOR = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);

  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) {
        done = true;
      }
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          event.window.windowID == SDL_GetWindowID(impl_->window)) {
        done = true;
      }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    drawPlots();
    drawControls();
    ImGui::Render();

    const auto& io = ImGui::GetIO();
    SDL_SetRenderScale(impl_->renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(impl_->renderer, BK_COLOR.x, BK_COLOR.y, BK_COLOR.z, BK_COLOR.w);
    SDL_RenderClear(impl_->renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), impl_->renderer);
    SDL_RenderPresent(impl_->renderer);
  }
}

//-------------------------------------------------------------------------------------------------
void Monitor::recv(const std::vector<grape::probe::Signal>& signals,
                   std::span<const std::byte> frame) {
  const std::unique_lock signals_lock(impl_->signals_lock);
  if (impl_->signals_buffer == nullptr) {
    static constexpr auto BUFFER_MAX_SIZE = 2000U;
    impl_->signals_buffer = std::make_unique<ScrollingBuffer>(BUFFER_MAX_SIZE, signals);
    const std::unique_lock ctrl_lock(impl_->controllables_lock);
    impl_->controllables = std::make_unique<Controllables>(signals, frame);
  }
  impl_->signals_buffer->addFrame(frame);
}

//-------------------------------------------------------------------------------------------------
void Monitor::drawPlots() {
  static constexpr auto PLOT_SIZE = ImVec2(-1, 150);
  static constexpr auto AXIS_FLAGS_X = ImPlotAxisFlags_None;
  static constexpr auto AXIS_FLAGS_Y = ImPlotAxisFlags_None | ImPlotAxisFlags_AutoFit;
  static constexpr auto PLOT_FILL_ALPHA = 0.5F;
  static constexpr auto MAX_HISTORY_SECONDS = 30.F;
  static constexpr auto MIN_HISTORY_SECONDS = 1.F;

  const ImGuiIO& io = ImGui::GetIO();

  static auto ts = 0.;
  ts += static_cast<double>(io.DeltaTime);

  const std::shared_lock lock(impl_->signals_lock);
  if (not impl_->signals_buffer) {
    return;
  }

  ImGui::Begin("Plots");
  static auto history = MIN_HISTORY_SECONDS;
  ImGui::SliderFloat("History", &history, 1, MAX_HISTORY_SECONDS, "%.1f s");

  const auto& signals_info = impl_->signals_buffer->signalsInfo();
  for (auto signal_number = 0U; signal_number < signals_info.size(); ++signal_number) {
    const auto signal = signals_info.at(signal_number);
    const auto* signal_name = signal.name.cStr();
    if (signal.role == Signal::Role::Timestamp) {
      continue;
    }
    if (signal.role == Signal::Role::Watch) {
      if (ImPlot::BeginPlot(signal_name, PLOT_SIZE)) {
        ImPlot::SetupAxes(nullptr, nullptr, AXIS_FLAGS_X, AXIS_FLAGS_Y);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, PLOT_FILL_ALPHA);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
        // TODO(vilas): X axis limits must come from time in data history rather than t
        ImPlot::SetupAxisLimits(ImAxis_X1, ts - static_cast<double>(history), ts, ImGuiCond_Always);
        for (auto trace_number = 0U; trace_number < signal.num_elements; ++trace_number) {
          auto trace_name = std::string(signal_name);
          if (signal.num_elements > 1) {
            trace_name += "[" + std::to_string(trace_number) + "]";
          }
          impl_->signals_buffer->markTrace({ .index = signal_number, .sub_index = trace_number });
          ImPlot::PlotLineG(trace_name.c_str(), signalDataGetter, impl_->signals_buffer.get(),
                            static_cast<int>(impl_->signals_buffer->length()),
                            /*ImPlotLineFlags*/ 0);
          // TODO(vilas): use PlotLineEx directly when reading off separate timestamp and signals
        }
        ImPlot::EndPlot();
      }
    }
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  ImGui::Text("FPS: %.1f", static_cast<double>(io.Framerate));
  ImGui::End();
}

//-------------------------------------------------------------------------------------------------
void Monitor::drawControls() {
  const std::shared_lock lock(impl_->controllables_lock);
  if (not impl_->controllables) {
    return;
  }

  // TODO(vilas): show current values with ImGuiInputTextFlags_ReadOnly

  ImGui::Begin("Controls");

  for (auto& item : impl_->controllables->items()) {
    const auto& signal_info = item.info;
    auto* data_ptr = item.data.data();
    const auto signal_name = std::string(signal_info.name.str());
    ImGui::AlignTextToFramePadding();

    ImGui::Text("%s", signal_name.c_str());  // NOLINT(cppcoreguidelines-pro-type-vararg)

    ImGui::SameLine();
    const auto trace_name = std::string("##") + signal_name;
    ImGui::InputScalarN(trace_name.c_str(), toImGuiDataType(signal_info.type), data_ptr,
                        static_cast<int>(signal_info.num_elements));

    ImGui::SameLine();
    const auto button_name = std::string("Apply##") + signal_name;
    const auto is_value_changed = ImGui::Button(button_name.c_str());

    if (is_value_changed) {
      if (impl_->sender != nullptr) {
        impl_->sender(signal_name, item.data);
      }
    }
  }  // each signal
  ImGui::End();
}

}  // namespace grape::probe
