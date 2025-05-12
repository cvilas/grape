//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>
#include <stdexcept>
#include <string>

#include <SDL3/SDL.h>

#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"
#include "implot.h"

//=================================================================================================
// Implot demo example.
class ImplotExample {
public:
  ImplotExample();
  ~ImplotExample();
  void run();

  ImplotExample(const ImplotExample&) = delete;
  ImplotExample(ImplotExample&&) = delete;
  auto operator=(const ImplotExample&) = delete;
  auto operator=(ImplotExample&&) = delete;

private:
  void demoRealtimePlot();
  SDL_Window* window_;
  SDL_Renderer* renderer_;
};

//=================================================================================================
auto main() -> int {
  try {
    ImplotExample ex;
    ex.run();
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    std::ignore = std::fputs("Exception\n", stderr);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
ImplotExample::ImplotExample() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw std::runtime_error(std::format("Error: SDL_Init(): {}", SDL_GetError()));
  }

  // Create window with graphics context

  static constexpr auto WIN_W = 1280;
  static constexpr auto WIN_H = 720;
  const auto win_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  window_ = SDL_CreateWindow("Implot Example", WIN_W, WIN_H, win_flags);
  if (window_ == nullptr) {
    throw std::runtime_error(std::format("Error: SDL_CreateWindow(): {}", SDL_GetError()));
  }
  renderer_ = SDL_CreateRenderer(window_, nullptr);
  SDL_SetRenderVSync(renderer_, 1);
  if (renderer_ == nullptr) {
    throw std::runtime_error(std::format("Error: SDL_CreateRenderer(): {}", SDL_GetError()));
  }
  SDL_ShowWindow(window_);

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  if (nullptr == ImGui::CreateContext()) {
    std::println("Error: ImGui::CreateContext");
  }
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // NOLINT(hicpp-signed-bitwise)

  ImGui::StyleColorsDark();

  if (not ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_)) {
    std::println("Error: ImGui_ImplSDL3_InitForSDLRenderer");
  }

  if (not ImGui_ImplSDLRenderer3_Init(renderer_)) {
    std::println("Error: ImGui_ImplSDLRenderer3_Init");
  }

  if (ImPlot::CreateContext() == nullptr) {
    std::println("Error: ImPlot::CreateContext");
  }
}

//-------------------------------------------------------------------------------------------------
ImplotExample::~ImplotExample() {
  ImPlot::DestroyContext();
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

//-------------------------------------------------------------------------------------------------
void ImplotExample::run() {
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
          event.window.windowID == SDL_GetWindowID(window_)) {
        done = true;
      }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    demoRealtimePlot();
    ImGui::Render();
    const auto& io = ImGui::GetIO();
    SDL_SetRenderScale(renderer_, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(renderer_, BK_COLOR.x, BK_COLOR.y, BK_COLOR.z, BK_COLOR.w);
    SDL_RenderClear(renderer_);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
  }
}

//-------------------------------------------------------------------------------------------------
// utility structure for realtime plot
struct ScrollingBuffer {
private:
  int max_size_;
  int offset_{ 0 };
  ImVector<ImVec2> data_;

public:
  explicit ScrollingBuffer(int max_size) : max_size_(max_size) {
    data_.reserve(max_size_);
  }

  void addPoint(float x_val, float y_val) {
    if (data_.size() < max_size_) {
      data_.push_back(ImVec2(x_val, y_val));
    } else {
      data_[offset_] = ImVec2(x_val, y_val);
      offset_ = (offset_ + 1) % max_size_;
    }
  }

  void erase() {
    if (not data_.empty()) {
      data_.shrink(0);
      offset_ = 0;
    }
  }

  [[nodiscard]] auto data() const -> const ImVector<ImVec2>& {
    return data_;
  }

  [[nodiscard]] auto offset() const -> int {
    return offset_;
  }
};

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ImplotExample::demoRealtimePlot() {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-vararg)
  ImGui::Begin("Plots");
  ImGui::BulletText("Move your mouse to change the data!");
  ImGui::BulletText("This example assumes 60 FPS. Higher FPS requires larger buffer size.");

  static ScrollingBuffer mxbf(2000);
  static ScrollingBuffer mybf(2000);
  const ImVec2 mouse = ImGui::GetMousePos();
  const ImGuiIO& io = ImGui::GetIO();
  static auto ts = 0.F;
  ts += io.DeltaTime;
  mxbf.addPoint(ts, mouse.x * 0.0005F);
  mybf.addPoint(ts, mouse.y * 0.0005F);

  static float history = 10.0F;
  ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");

  const ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

  if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, 150))) {
    ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(ts - history), static_cast<double>(ts),
                            ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5F);
    const auto& mx = mxbf.data();
    const auto& my = mybf.data();
    ImPlot::PlotLine("Mouse X", &mx[0].x, &mx[0].y, mx.size(), 0, mxbf.offset(), 2 * sizeof(float));
    ImPlot::PlotLine("Mouse Y", &my[0].x, &my[0].y, my.size(), 0, mybf.offset(), 2 * sizeof(float));
    ImPlot::EndPlot();
  }
  ImGui::Text("FPS: %.1f", static_cast<double>(io.Framerate));
  ImGui::End();
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-vararg)
}
