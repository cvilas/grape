//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>
#include <stdexcept>
#include <string>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
  static void glfwEerrorCb(int error, const char* description);
  void demoRealtimePlot();
  GLFWwindow* window_;
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
void ImplotExample::glfwEerrorCb(int error, const char* description) {
  std::println(stderr, "GLFW Error {}: {}", error, description);
}

//-------------------------------------------------------------------------------------------------
ImplotExample::ImplotExample() {
  glfwSetErrorCallback(ImplotExample::glfwEerrorCb);
  if (GLFW_FALSE == glfwInit()) {
    throw std::format("Error: glfwInit");
  }

  // Decide GL+GLSL versions
#if defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Create window with graphics context
  static constexpr auto WIN_W = 1280;
  static constexpr auto WIN_H = 720;
  window_ = glfwCreateWindow(WIN_W, WIN_H, "Implot example", nullptr, nullptr);
  if (window_ == nullptr) {
    glfwTerminate();
    throw std::format("Error: glfwCreateWindow");
  }
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);  // Enable vsync

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  if (nullptr == ImGui::CreateContext()) {
    std::println("Error: ImGui::CreateContext");
  }
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // NOLINT(hicpp-signed-bitwise)

  ImGui::StyleColorsDark();

  if (not ImGui_ImplGlfw_InitForOpenGL(window_, true)) {
    std::println("Error: ImGui_ImplGlfw_InitForOpenGL");
  }

  if (not ImGui_ImplOpenGL3_Init(glsl_version)) {
    std::println("Error: ImGui_ImplOpenGL3_Init");
  }

  if (ImPlot::CreateContext() == nullptr) {
    std::println("Error: ImPlot::CreateContext");
  }
}

//-------------------------------------------------------------------------------------------------
ImplotExample::~ImplotExample() {
  ImPlot::DestroyContext();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window_);
  glfwTerminate();
}

//-------------------------------------------------------------------------------------------------
void ImplotExample::run() {
  static constexpr auto BK_COLOR = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  while (glfwWindowShouldClose(window_) == 0) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    demoRealtimePlot();
    ImGui::Render();

    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(BK_COLOR.x * BK_COLOR.w, BK_COLOR.y * BK_COLOR.w, BK_COLOR.z * BK_COLOR.w,
                 BK_COLOR.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);
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
