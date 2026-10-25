//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <cstdio>

#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_sdlgpu3.h>
#include <implot/implot.h>

int main() {
  // Volatile pointers require the linker to resolve the GPU backend without a physical GPU.
  auto* volatile init = &ImGui_ImplSDLGPU3_Init;
  auto* volatile prepare = &ImGui_ImplSDLGPU3_PrepareDrawData;
  auto* volatile render = &ImGui_ImplSDLGPU3_RenderDrawData;
  auto* volatile shutdown = &ImGui_ImplSDLGPU3_Shutdown;
  auto* volatile platform = &ImGui_ImplSDL3_InitForSDLGPU;
  if (init == nullptr || prepare == nullptr || render == nullptr || shutdown == nullptr ||
      platform == nullptr) {
    std::fputs("SDL_GPU backend symbols are missing\n", stderr);
    return 1;
  }
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::SetImGuiContext(ImGui::GetCurrentContext());
  ImPlot::CreateContext();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  return 0;
}
