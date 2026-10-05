//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

// Renderer and interaction for the gaussian splat viewer.
//
// How a frame is drawn (see docs/gaussian_splatting.md for the theory):
//
//   1. At load time, each splat's 3D covariance is computed once on the CPU (toGpuSplat) and the
//      whole scene is uploaded to a GPU storage buffer. It never changes after that.
//   2. Whenever the camera moves, a worker thread sorts the splats far-to-near (AsyncSorter). The
//      resulting list of indices is uploaded to a second storage buffer.
//   3. One instanced draw call renders a 4-vertex quad per splat, in sorted order. The vertex
//      shader (shaders/splat.vert) projects the 3D gaussian to a 2D ellipse on screen and sizes
//      the quad to cover it. The fragment shader (shaders/splat.frag) evaluates the gaussian at
//      each pixel and outputs its colour weighted by opacity.
//   4. The GPU's fixed-function blender composites the quads with the 'over' operator. Because
//      they arrive far-to-near, each one correctly covers what is behind it.
//
// There is no depth buffer and no mesh. Everything else in this file is window and input handling.

#include "grape/splat/viewer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <utility>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_video.h>

#include "async_sorter.h"
#include "gpu.h"
#include "grape/exception.h"

// SPIR-V shader binaries generated at build time. See CMakeLists.txt
#include "splat_frag_spv.h"
#include "splat_vert_spv.h"

namespace {

using grape::splat::ExaminerCamera;
using grape::splat::Mat4;
using grape::splat::Splat;
using grape::splat::Vec3;

//=================================================================================================
// GPU data layouts. Must match declarations in shaders/splat.vert (std430/std140 rules)
//=================================================================================================

//-------------------------------------------------------------------------------------------------
/// One splat, as the vertex shader sees it. The covariance replaces scale and rotation, because
/// that is what projection needs (see covariance()). It is symmetric, so 6 numbers suffice.
struct GpuSplat {
  std::array<float, 3> position{};
  std::uint32_t rgba{ 0 };       // RGBA8 packed, R in the least significant byte
  std::array<float, 3> cov_a{};  // Σxx, Σxy, Σxz
  float pad0{ 0.F };             // std430 aligns vec3 to 16 bytes
  std::array<float, 3> cov_b{};  // Σyy, Σyz, Σzz
  float pad1{ 0.F };
};
static_assert(sizeof(GpuSplat) == 48);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
/// Per-frame camera parameters
struct CameraUniforms {
  std::array<float, 16> view{};  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
  std::array<float, 2> focal{};
  std::array<float, 2> viewport{};
};
static_assert(sizeof(CameraUniforms) == 80);  // NOLINT(cppcoreguidelines-avoid-magic-numbers)

//-------------------------------------------------------------------------------------------------
auto toGpuSplat(const Splat& splat) -> GpuSplat {
  const auto cov = covariance(splat);
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  const auto rgba = static_cast<std::uint32_t>(splat.color.r) |
                    (static_cast<std::uint32_t>(splat.color.g) << 8U) |
                    (static_cast<std::uint32_t>(splat.color.b) << 16U) |
                    (static_cast<std::uint32_t>(splat.color.a) << 24U);
  return {
    .position = { splat.position.x, splat.position.y, splat.position.z },
    .rgba = rgba,
    .cov_a = { cov.at(0), cov.at(1), cov.at(2) },
    .pad0 = 0.F,
    .cov_b = { cov.at(3), cov.at(4), cov.at(5) },
    .pad1 = 0.F,
  };
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

//-------------------------------------------------------------------------------------------------
auto createPipeline(SDL_GPUDevice* device GRAPE_LIFETIMEBOUND, SDL_Window* window)
    -> grape::splat::gpu::Pipeline {
  using grape::splat::gpu::createShader;
  // Vertex shader reads 2 storage buffers (splats, order) and 1 uniform buffer (camera)
  const auto vert = createShader(device, SPLAT_VERT_SPV, SDL_GPU_SHADERSTAGE_VERTEX, 2, 1);
  const auto frag = createShader(device, SPLAT_FRAG_SPV, SDL_GPU_SHADERSTAGE_FRAGMENT, 0, 0);

  // The 'over' operator for premultiplied colour: result = src + (1 - src_alpha) * dst.
  // Drawn far-to-near, each splat lets through (1 - alpha) of whatever is behind it.
  const auto color_target = SDL_GPUColorTargetDescription{
    .format = SDL_GetGPUSwapchainTextureFormat(device, window),
    .blend_state = {
        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .color_blend_op = SDL_GPU_BLENDOP_ADD,
        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE,
        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
        .color_write_mask = 0,
        .enable_blend = true,
        .enable_color_write_mask = false,
        .padding1 = 0,
        .padding2 = 0,
    },
  };
  // No vertex buffers: the vertex shader computes quad corners from gl_VertexIndex. No depth
  // buffer: visibility comes entirely from the draw order.
  auto info = SDL_GPUGraphicsPipelineCreateInfo{};
  info.vertex_shader = vert.get();
  info.fragment_shader = frag.get();
  info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
  info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
  info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
  info.target_info.color_target_descriptions = &color_target;
  info.target_info.num_color_targets = 1;

  auto* pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
  if (pipeline == nullptr) {
    grape::panic(std::format("SDL_CreateGPUGraphicsPipeline: {}", SDL_GetError()));
  }
  return { device, pipeline };
}

//-------------------------------------------------------------------------------------------------
/// @return Bit mask for a mouse button index (SDL_BUTTON_LEFT, etc)
constexpr auto buttonMask(std::uint8_t button) -> SDL_MouseButtonFlags {
  return (button > 0U) ? (SDL_MouseButtonFlags{ 1U } << (button - 1U)) : 0U;
}

//-------------------------------------------------------------------------------------------------
/// @return true if any of the modifier keys in flags are set in mods
constexpr auto hasModifier(SDL_Keymod mods, SDL_Keymod flags) -> bool {
  return (static_cast<unsigned>(mods) & static_cast<unsigned>(flags)) != 0U;
}

}  // namespace

namespace grape::splat {

//=================================================================================================
struct Viewer::Impl {
  /// Navigation action selected by the mouse buttons and modifier keys held down
  enum class DragMode : std::uint8_t { None, Rotate, Pan, Dolly };

  explicit Impl(const Config& cfg);

  void setScene(std::vector<Splat> splats);
  void handleEvent(const SDL_Event& event);
  void onMouseButton(const SDL_MouseButtonEvent& event, bool down);
  void onMouseMotion(const SDL_MouseMotionEvent& event);
  void onKey(const SDL_KeyboardEvent& event);
  void render();
  void updateSpin(double dt);
  void viewAll();
  [[nodiscard]] auto pick(float px, float py) const -> std::optional<Vec3>;
  [[nodiscard]] auto windowSize() const -> std::pair<float, float>;
  [[nodiscard]] auto toNormalised(float px, float py) const -> std::pair<float, float>;
  [[nodiscard]] static auto dragMode(SDL_MouseButtonFlags buttons, SDL_Keymod mods) -> DragMode;

  static constexpr auto SPIN_TIMEOUT = std::chrono::milliseconds(100);
  static constexpr auto MIN_SPIN_RATE = 0.05F;  // rad/s
  static constexpr auto DOLLY_GAIN = 0.005F;    // per pixel of motion
  static constexpr auto WHEEL_DOLLY_FACTOR = 0.9F;
  static constexpr auto SEEK_APPROACH = 0.5F;
  static constexpr auto PICK_RADIUS_PX = 5.F;
  static constexpr auto PICK_MIN_ALPHA = std::uint8_t{ 32 };

  Config config;
  gpu::SdlVideo sdl_video;
  std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> window{ nullptr, SDL_DestroyWindow };
  std::unique_ptr<SDL_GPUDevice, void (*)(SDL_GPUDevice*)> device{ nullptr, SDL_DestroyGPUDevice };
  std::unique_ptr<gpu::WindowClaim> claim;
  gpu::Pipeline pipeline;
  gpu::Buffer splat_buffer;            // GpuSplat per splat. Uploaded once per scene
  gpu::Buffer order_buffer;            // Splat indices, far to near. Uploaded after every sort
  gpu::TransferBuffer order_transfer;  // Staging area for order_buffer uploads
  std::uint32_t draw_count{ 0 };       // Number of splats in order_buffer

  std::shared_ptr<const std::vector<Splat>> scene;
  std::unique_ptr<AsyncSorter> sorter;
  std::optional<Mat4> requested_view;

  ExaminerCamera camera;
  ExaminerCamera home;
  bool is_open{ true };
  bool seek_mode{ false };
  SDL_MouseButtonFlags buttons{ 0 };
  DragMode drag{ DragMode::None };
  std::chrono::steady_clock::time_point last_motion_time;
  std::chrono::steady_clock::time_point last_frame_time{ std::chrono::steady_clock::now() };
  Vec3 spin_axis;
  float spin_rate{ 0.F };  // rad/s. Zero when not spinning
};

//-------------------------------------------------------------------------------------------------
/// Map mouse buttons and modifier keys to an examiner viewer action
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Viewer::Impl::dragMode(SDL_MouseButtonFlags buttons, SDL_Keymod mods) -> DragMode {
  const auto left = (buttons & buttonMask(SDL_BUTTON_LEFT)) != 0U;
  const auto middle = (buttons & buttonMask(SDL_BUTTON_MIDDLE)) != 0U;
  const auto right = (buttons & buttonMask(SDL_BUTTON_RIGHT)) != 0U;
  const auto ctrl = hasModifier(mods, SDL_KMOD_CTRL);
  const auto shift = hasModifier(mods, SDL_KMOD_SHIFT);
  if (right or (left and middle) or (middle and ctrl) or (left and ctrl and shift)) {
    return DragMode::Dolly;
  }
  if (middle or (left and (ctrl or shift))) {
    return DragMode::Pan;
  }
  if (left) {
    return DragMode::Rotate;
  }
  return DragMode::None;
}

//-------------------------------------------------------------------------------------------------
Viewer::Impl::Impl(const Config& cfg) : config(cfg), camera(cfg.fov_y), home(cfg.fov_y) {
  window.reset(SDL_CreateWindow(config.title.c_str(), config.width, config.height,
                                SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY));
  if (window == nullptr) {
    panic(std::format("SDL_CreateWindow: {}", SDL_GetError()));
  }
  static constexpr auto DEBUG_MODE = false;
  // NOLINTNEXTLINE(bugprone-signed-bitwise) - SDL flag macro
  device.reset(SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, DEBUG_MODE, nullptr));
  if (device == nullptr) {
    panic(std::format("SDL_CreateGPUDevice (SPIR-V/Vulkan required): {}", SDL_GetError()));
  }
  claim = std::make_unique<gpu::WindowClaim>(device.get(), window.get());
  pipeline = createPipeline(device.get(), window.get());
  setScene({});
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::setScene(std::vector<Splat> splats) {
  sorter.reset();
  requested_view.reset();
  draw_count = 0;
  splat_buffer.reset();
  order_buffer.reset();
  order_transfer.reset();
  scene = std::make_shared<const std::vector<Splat>>(std::move(splats));
  viewAll();
  home = camera;
  if (scene->empty()) {
    return;
  }

  auto gpu_splats = std::vector<GpuSplat>(scene->size());
  std::ranges::transform(*scene, gpu_splats.begin(), toGpuSplat);
  const auto splat_bytes = std::as_bytes(std::span{ gpu_splats });
  const auto order_size = scene->size() * sizeof(std::uint32_t);
  splat_buffer = gpu::createStorageBuffer(device.get(), splat_bytes.size());
  order_buffer = gpu::createStorageBuffer(device.get(), order_size);
  order_transfer = gpu::createTransferBuffer(device.get(), order_size);

  auto* cmd = gpu::acquireCommandBuffer(device.get());
  // Transfer buffer is only needed until the upload completes. SDL defers its release until then.
  const auto splat_transfer = gpu::createTransferBuffer(device.get(), splat_bytes.size());
  gpu::upload(device.get(), cmd, splat_transfer.get(), splat_buffer.get(), splat_bytes);
  gpu::submit(cmd);
  sorter = std::make_unique<AsyncSorter>(scene);
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::viewAll() {
  spin_rate = 0.F;
  const auto bounds = computeBounds(*scene);
  camera.viewAll(bounds.center, (bounds.radius > 0.F) ? bounds.radius : 1.F, config.up);
}

//-------------------------------------------------------------------------------------------------
auto Viewer::Impl::windowSize() const -> std::pair<float, float> {
  auto width = 1;
  auto height = 1;
  if (not SDL_GetWindowSize(window.get(), &width, &height)) {
    return { 1.F, 1.F };
  }
  return { static_cast<float>(std::max(width, 1)), static_cast<float>(std::max(height, 1)) };
}

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Viewer::Impl::toNormalised(float px, float py) const -> std::pair<float, float> {
  const auto [width, height] = windowSize();
  const auto half_width = width / 2.F;
  const auto half_height = height / 2.F;
  return { (px - half_width) / half_width, (half_height - py) / half_height };
}

//-------------------------------------------------------------------------------------------------
/// Find the scene point under a pixel, for 'seek'. There is no surface to intersect, so this picks
/// the nearest reasonably opaque splat whose projected centre lies within its own size (or a few
/// pixels) of the pointer. A brute force scan is fast enough for a one-off click.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Viewer::Impl::pick(float px, float py) const -> std::optional<Vec3> {
  const auto [width, height] = windowSize();
  const auto view = camera.viewMatrix();
  const auto focal = camera.focalLengthPx(height);
  const auto target_x = px - (width / 2.F);
  const auto target_y = (height / 2.F) - py;

  auto best = std::optional<Vec3>{};
  auto best_depth = std::numeric_limits<float>::max();
  for (const auto& splat : *scene) {
    if (splat.color.a < PICK_MIN_ALPHA) {
      continue;
    }
    const auto pt = transformPoint(view, splat.position);
    const auto depth = -pt.z;
    if (depth <= 0.F or depth >= best_depth) {
      continue;
    }
    const auto dx = (focal * pt.x / depth) - target_x;
    const auto dy = (focal * pt.y / depth) - target_y;
    const auto max_scale = std::max({ splat.scale.x, splat.scale.y, splat.scale.z });
    const auto radius = std::max(PICK_RADIUS_PX, focal * max_scale / depth);
    if ((dx * dx) + (dy * dy) <= radius * radius) {
      best = splat.position;
      best_depth = depth;
    }
  }
  return best;
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::handleEvent(const SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      is_open = false;
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      onMouseButton(event.button, true);
      break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      onMouseButton(event.button, false);
      break;
    case SDL_EVENT_MOUSE_MOTION:
      onMouseMotion(event.motion);
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      spin_rate = 0.F;
      camera.dolly(std::pow(WHEEL_DOLLY_FACTOR, event.wheel.y));
      break;
    case SDL_EVENT_KEY_DOWN:
      onKey(event.key);
      break;
    default:
      break;
  }
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::onMouseButton(const SDL_MouseButtonEvent& event, bool down) {
  const auto mask = buttonMask(event.button);
  if (down) {
    spin_rate = 0.F;
    if (seek_mode and event.button == SDL_BUTTON_LEFT) {
      seek_mode = false;
      if (const auto point = pick(event.x, event.y); point) {
        camera.seek(*point, SEEK_APPROACH);
      }
      return;
    }
    buttons |= mask;
  } else {
    // Keep spinning if the trackball was released while in motion
    const auto recently_moved =
        (std::chrono::steady_clock::now() - last_motion_time) < SPIN_TIMEOUT;
    if (drag != DragMode::Rotate or not recently_moved) {
      spin_rate = 0.F;
    }
    buttons &= ~mask;
  }
  drag = dragMode(buttons, SDL_GetModState());
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::onMouseMotion(const SDL_MouseMotionEvent& event) {
  drag = dragMode(buttons, SDL_GetModState());
  const auto now = std::chrono::steady_clock::now();
  const auto dt = std::chrono::duration<float>(now - last_motion_time).count();
  last_motion_time = now;

  switch (drag) {
    case DragMode::None:
      break;
    case DragMode::Rotate: {
      const auto [width, height] = windowSize();
      const auto [x0, y0] = toNormalised(event.x - event.xrel, event.y - event.yrel);
      const auto [x1, y1] = toNormalised(event.x, event.y);
      const auto delta = camera.trackball(x0, y0, x1, y1, width / height);
      // Track angular velocity for spinning after release
      const auto angle = 2.F * std::acos(std::clamp(delta.w, -1.F, 1.F));
      spin_axis = Vec3{ .x = delta.x, .y = delta.y, .z = delta.z };
      spin_rate = (dt > 0.F and length(spin_axis) > 0.F) ? angle / dt : 0.F;
    } break;
    case DragMode::Pan:
      camera.pan(event.xrel, event.yrel, windowSize().second);
      break;
    case DragMode::Dolly:
      camera.dolly(std::exp(event.yrel * DOLLY_GAIN));
      break;
  }
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::onKey(const SDL_KeyboardEvent& event) {
  switch (event.key) {
    case SDLK_S:
      seek_mode = true;
      break;
    case SDLK_ESCAPE:
      seek_mode = false;
      break;
    case SDLK_V:
      viewAll();
      break;
    case SDLK_H:
      spin_rate = 0.F;
      if (hasModifier(event.mod, SDL_KMOD_SHIFT)) {
        home = camera;
      } else {
        camera = home;
      }
      break;
    default:
      break;
  }
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::updateSpin(double dt) {
  // Not spinning while the user is still dragging
  if (spin_rate < MIN_SPIN_RATE or drag != DragMode::None) {
    return;
  }
  camera.spin(fromAxisAngle(spin_axis, spin_rate * static_cast<float>(dt)));
}

//-------------------------------------------------------------------------------------------------
void Viewer::Impl::render() {
  const auto now = std::chrono::steady_clock::now();
  updateSpin(std::chrono::duration<double>(now - last_frame_time).count());
  last_frame_time = now;

  // Step 2 (see top of file): Ask for a re-sort when the view changes, and collect the result
  // when it is ready. Until then, keep drawing with the previous order. It is slightly stale
  // during fast motion, but the error is rarely visible.
  const auto view = camera.viewMatrix();
  if (sorter != nullptr and requested_view != view) {
    sorter->request(view);
    requested_view = view;
  }
  auto new_order = (sorter != nullptr) ? sorter->take() : std::nullopt;

  auto* cmd = gpu::acquireCommandBuffer(device.get());

  // Upload the new draw order, if any
  if (new_order) {
    draw_count = static_cast<std::uint32_t>(new_order->size());
    if (draw_count > 0) {
      gpu::upload(device.get(), cmd, order_transfer.get(), order_buffer.get(),
                  std::as_bytes(std::span{ *new_order }));
    }
  }

  SDL_GPUTexture* swapchain = nullptr;
  auto width = std::uint32_t{ 0 };
  auto height = std::uint32_t{ 0 };
  if (not SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window.get(), &swapchain, &width, &height)) {
    SDL_CancelGPUCommandBuffer(cmd);
    panic(std::format("SDL_WaitAndAcquireGPUSwapchainTexture: {}", SDL_GetError()));
  }
  if (swapchain == nullptr) {
    // Window is minimised or otherwise not presentable. Still submit any pending uploads.
    std::ignore = SDL_SubmitGPUCommandBuffer(cmd);
    return;
  }

  // Step 3: Everything the vertex shader needs to know about the camera
  const auto focal = camera.focalLengthPx(static_cast<float>(height));
  const auto uniforms = CameraUniforms{
    .view = view.m,
    .focal = { focal, focal },
    .viewport = { static_cast<float>(width), static_cast<float>(height) },
  };
  SDL_PushGPUVertexUniformData(cmd, 0, &uniforms, sizeof(uniforms));

  auto target = SDL_GPUColorTargetInfo{};
  target.texture = swapchain;
  target.clear_color = SDL_FColor{ .r = 0.F, .g = 0.F, .b = 0.F, .a = 1.F };
  target.load_op = SDL_GPU_LOADOP_CLEAR;
  target.store_op = SDL_GPU_STOREOP_STORE;
  auto* pass = SDL_BeginGPURenderPass(cmd, &target, 1, nullptr);
  if (draw_count > 0) {
    // One instance per splat, 4 vertices (a triangle strip quad) per instance. Instance i draws
    // splat order[i], so splats are drawn far to near.
    SDL_BindGPUGraphicsPipeline(pass, pipeline.get());
    const auto buffers = std::array{ splat_buffer.get(), order_buffer.get() };
    SDL_BindGPUVertexStorageBuffers(pass, 0, buffers.data(), buffers.size());
    static constexpr auto VERTICES_PER_SPLAT = 4U;
    SDL_DrawGPUPrimitives(pass, VERTICES_PER_SPLAT, draw_count, 0, 0);
  }
  SDL_EndGPURenderPass(pass);

  gpu::submit(cmd);
}

//=================================================================================================
Viewer::Viewer(const Config& config) : impl_(std::make_unique<Impl>(config)) {
}

//-------------------------------------------------------------------------------------------------
Viewer::~Viewer() = default;

//-------------------------------------------------------------------------------------------------
void Viewer::setScene(std::vector<Splat> splats) {
  impl_->setScene(std::move(splats));
}

//-------------------------------------------------------------------------------------------------
auto Viewer::processEvents() -> bool {
  auto event = SDL_Event{};
  while (SDL_PollEvent(&event)) {
    impl_->handleEvent(event);
  }
  return impl_->is_open;
}

//-------------------------------------------------------------------------------------------------
void Viewer::render() {
  impl_->render();
}

}  // namespace grape::splat
