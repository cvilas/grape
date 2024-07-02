//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>
#include <stdexcept>
#include <string>

#include <SDL3/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "implot.h"

//=================================================================================================
// Demo example that uses 2D rendering surface for plots.
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
  } catch (const std::string& ex) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = std::fprintf(stderr, "%s\n", ex.c_str());
    return EXIT_FAILURE;
  } catch (const std::exception& ex) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = std::fprintf(stderr, "%s\n", ex.what());
    return EXIT_FAILURE;
  } catch (...) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = std::fprintf(stderr, "Exception\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
ImplotExample::ImplotExample() {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    throw std::format("Error: SDL_Init: {}", SDL_GetError());
  }

  // Create window with graphics context
  const auto window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
  window_ = SDL_CreateWindow("Implot example", 1280, 720, window_flags);
  if (window_ == nullptr) {
    throw std::format("Error: SDL_CreateWindow(): {}", SDL_GetError());
  }
  renderer_ = SDL_CreateRenderer(window_, nullptr);
  if (renderer_ == nullptr) {
    throw std::format("Error: SDL_CreateRenderer(): {}", SDL_GetError());
  }
  if (0 != SDL_SetRenderVSync(renderer_, 1)) {
    std::println("Error: SDL_SetRenderVSync: {}", SDL_GetError());
  }

  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  if (0 != SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED)) {
    std::println("Error: SDL_SetWindowPosition: {}", SDL_GetError());
  }
  if (0 != SDL_ShowWindow(window_)) {
    std::println("Error: SDL_ShowWindow: {}", SDL_GetError());
  }

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  if (nullptr == ImGui::CreateContext()) {
    std::println("Error: ImGui::CreateContext");
  }
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // NOLINT(hicpp-signed-bitwise)

  ImGui::StyleColorsDark();

  // Setup ImGui for SDL3
  if (not ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_)) {
    std::println("Error: ImGui_ImplSDL3_InitForSDLRenderer");
  }
  if (not ImGui_ImplSDLRenderer3_Init(renderer_)) {
    std::println("Error: ImGui_ImplSDLRenderer3_Init");
  }

  if (ImPlot::CreateContext() == nullptr) {
    std::println("Error: ImPlot::CreateContext");
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

//-------------------------------------------------------------------------------------------------
ImplotExample::~ImplotExample() {
  ImPlot::DestroyContext();
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  if (renderer_ != nullptr) {
    SDL_DestroyRenderer(renderer_);
  }
  if (window_ != nullptr) {
    SDL_DestroyWindow(window_);
  }
  SDL_Quit();
}

//-------------------------------------------------------------------------------------------------
void ImplotExample::run() {
  bool is_running = true;

  while (is_running) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) {
        is_running = false;
      }
      if ((event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) &&
          (event.window.windowID == SDL_GetWindowID(window_))) {
        is_running = false;
      }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    demoRealtimePlot();
    ImGui::Render();
    static constexpr auto BK_COLOR = std::array<Uint8, 4>{ 114, 140, 153, 255 };
    SDL_SetRenderDrawColor(renderer_, BK_COLOR[0], BK_COLOR[1], BK_COLOR[2], BK_COLOR[3]);
    SDL_RenderClear(renderer_);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);

    if (0 != SDL_RenderPresent(renderer_)) {
      std::println("Error: SDL_RenderPresent: {}", SDL_GetError());
      is_running = false;
    }
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

  void addPoint(float x, float y) {
    if (data_.size() < max_size_) {
      data_.push_back(ImVec2(x, y));
    } else {
      data_[offset_] = ImVec2(x, y);
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
void ImplotExample::demoRealtimePlot() {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-vararg)
  ImGui::Begin("Plots");
  ImGui::BulletText("Move your mouse to change the data!");
  ImGui::BulletText("This example assumes 60 FPS. Higher FPS requires larger buffer size.");

  static ScrollingBuffer mxbf(2000);
  static ScrollingBuffer mybf(2000);
  const ImVec2 mouse = ImGui::GetMousePos();
  const ImGuiIO& io = ImGui::GetIO();
  static auto t = 0.f;
  t += io.DeltaTime;
  mxbf.addPoint(t, mouse.x * 0.0005f);
  mybf.addPoint(t, mouse.y * 0.0005f);

  static float history = 10.0f;
  ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");

  const ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

  if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, 150))) {
    ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t - history), static_cast<double>(t),
                            ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
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
