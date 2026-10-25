//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

#include <SDL3/SDL.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_sdlrenderer3.h>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <implot/implot.h>

namespace {
void require(bool condition, const char* operation) {
  if (!condition) {
    throw std::runtime_error(std::string(operation) + ": " + SDL_GetError());
  }
}

struct Session {
  Session() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::SetImGuiContext(ImGui::GetCurrentContext());
    ImPlot::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
  }
  ~Session() {
    if (ImGui::GetIO().BackendRendererUserData != nullptr) {
      ImGui_ImplSDLRenderer3_Shutdown();
    }
    if (ImGui::GetIO().BackendPlatformUserData != nullptr) {
      ImGui_ImplSDL3_Shutdown();
    }
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
  }
  Session(const Session&) = delete;
  auto operator=(const Session&) -> Session& = delete;
};

void run() {
  const std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
      SDL_CreateWindow("GUI package smoke test", 640, 480, SDL_WINDOW_HIDDEN), SDL_DestroyWindow);
  require(window != nullptr, "SDL_CreateWindow");
  const std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
      SDL_CreateRenderer(window.get(), "software"), SDL_DestroyRenderer);
  require(renderer != nullptr, "SDL_CreateRenderer");
  const Session session;
  require(ImGui_ImplSDL3_InitForSDLRenderer(window.get(), renderer.get()), "ImGui_ImplSDL3_Init");
  require(ImGui_ImplSDLRenderer3_Init(renderer.get()), "ImGui_ImplSDLRenderer3_Init");
  require((ImGui::GetIO().BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset) != 0,
          "Renderer must support large ImPlot meshes");

  for (int frame = 0; frame < 3; ++frame) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Plot");
    std::string label = "package test";
    ImGui::InputText("Label", &label);
    if (ImPlot::BeginPlot("Line")) {
      constexpr double values[] = { 0.0, 1.0, 0.5 };
      const ImPlotSpec spec(ImPlotProp_FillAlpha, 0.5F, ImPlotProp_Offset, 1, ImPlotProp_Stride,
                            sizeof(double));
      ImPlot::PlotLine("values", values, 3, 1.0, 0.0, spec);
      ImPlot::EndPlot();
    }
    ImGui::End();
#ifdef TEST_DEMOS
    ImGui::ShowDemoWindow();
    ImPlot::ShowDemoWindow();
#endif
    for (int i = 0; i < 20000; ++i) {
      ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(1, 1), ImVec2(2, 2), IM_COL32_WHITE);
    }
    ImGui::Render();
    require(ImGui::GetDrawData()->TotalVtxCount > 65535,
            "Exercise more than 16-bit vertex indices");
    require(SDL_RenderClear(renderer.get()), "SDL_RenderClear");
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer.get());
    require(SDL_RenderPresent(renderer.get()), "SDL_RenderPresent");
  }
}
}  // namespace

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }
  try {
    run();
  } catch (const std::exception& error) {
    std::fprintf(stderr, "%s\n", error.what());
    SDL_Quit();
    return 1;
  }
  SDL_Quit();
  return 0;
}
