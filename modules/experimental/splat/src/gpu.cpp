//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "gpu.h"

#include <cstring>
#include <format>
#include <limits>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>

#include "grape/exception.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto checkedSize(std::size_t bytes) -> std::uint32_t {
  if (bytes > std::numeric_limits<std::uint32_t>::max()) {
    grape::panic(std::format("Buffer size {} bytes exceeds GPU limits", bytes));
  }
  return static_cast<std::uint32_t>(bytes);
}

}  // namespace

namespace grape::splat::gpu {

//-------------------------------------------------------------------------------------------------
SdlVideo::SdlVideo() {
  if (not SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    panic(std::format("SDL_InitSubSystem: {}", SDL_GetError()));
  }
}

//-------------------------------------------------------------------------------------------------
SdlVideo::~SdlVideo() {
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

//-------------------------------------------------------------------------------------------------
WindowClaim::WindowClaim(SDL_GPUDevice* device, SDL_Window* window)
  : device_(device), window_(window) {
  if (not SDL_ClaimWindowForGPUDevice(device_, window_)) {
    panic(std::format("SDL_ClaimWindowForGPUDevice: {}", SDL_GetError()));
  }
}

//-------------------------------------------------------------------------------------------------
WindowClaim::~WindowClaim() {
  SDL_ReleaseWindowFromGPUDevice(device_, window_);
}

//-------------------------------------------------------------------------------------------------
auto createShader(SDL_GPUDevice* device, std::span<const std::uint32_t> spirv,
                  SDL_GPUShaderStage stage, std::uint32_t num_storage_buffers,
                  std::uint32_t num_uniform_buffers) -> Shader {
  const auto info = SDL_GPUShaderCreateInfo{
    .code_size = spirv.size_bytes(),
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    .code = reinterpret_cast<const std::uint8_t*>(spirv.data()),
    .entrypoint = "main",
    // NOLINTNEXTLINE(bugprone-signed-bitwise) - SDL flag macro
    .format = SDL_GPU_SHADERFORMAT_SPIRV,
    .stage = stage,
    .num_samplers = 0,
    .num_storage_textures = 0,
    .num_storage_buffers = num_storage_buffers,
    .num_uniform_buffers = num_uniform_buffers,
    .props = 0,
  };
  auto* shader = SDL_CreateGPUShader(device, &info);
  if (shader == nullptr) {
    panic(std::format("SDL_CreateGPUShader: {}", SDL_GetError()));
  }
  return { device, shader };
}

//-------------------------------------------------------------------------------------------------
auto createStorageBuffer(SDL_GPUDevice* device, std::size_t size_bytes) -> Buffer {
  const auto info = SDL_GPUBufferCreateInfo{
    // NOLINTNEXTLINE(bugprone-signed-bitwise) - SDL flag macro
    .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
    .size = checkedSize(size_bytes),
    .props = 0,
  };
  auto* buffer = SDL_CreateGPUBuffer(device, &info);
  if (buffer == nullptr) {
    panic(std::format("SDL_CreateGPUBuffer: {}", SDL_GetError()));
  }
  return { device, buffer };
}

//-------------------------------------------------------------------------------------------------
auto createTransferBuffer(SDL_GPUDevice* device, std::size_t size_bytes) -> TransferBuffer {
  const auto info = SDL_GPUTransferBufferCreateInfo{
    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    .size = checkedSize(size_bytes),
    .props = 0,
  };
  auto* buffer = SDL_CreateGPUTransferBuffer(device, &info);
  if (buffer == nullptr) {
    panic(std::format("SDL_CreateGPUTransferBuffer: {}", SDL_GetError()));
  }
  return { device, buffer };
}

//-------------------------------------------------------------------------------------------------
void upload(SDL_GPUDevice* device, SDL_GPUCommandBuffer* cmd, SDL_GPUTransferBuffer* transfer,
            SDL_GPUBuffer* buffer, std::span<const std::byte> bytes) {
  // 'cycle = true' lets SDL hand out a fresh region if the GPU is still reading the previous one,
  // so that uploading every frame does not stall on frames in flight.
  static constexpr auto CYCLE = true;
  auto* mapped = SDL_MapGPUTransferBuffer(device, transfer, CYCLE);
  if (mapped == nullptr) {
    panic(std::format("SDL_MapGPUTransferBuffer: {}", SDL_GetError()));
  }
  std::memcpy(mapped, bytes.data(), bytes.size());
  SDL_UnmapGPUTransferBuffer(device, transfer);

  const auto src = SDL_GPUTransferBufferLocation{ .transfer_buffer = transfer, .offset = 0 };
  const auto dst = SDL_GPUBufferRegion{
    .buffer = buffer,
    .offset = 0,
    .size = checkedSize(bytes.size()),
  };
  auto* copy_pass = SDL_BeginGPUCopyPass(cmd);
  SDL_UploadToGPUBuffer(copy_pass, &src, &dst, CYCLE);
  SDL_EndGPUCopyPass(copy_pass);
}

//-------------------------------------------------------------------------------------------------
auto acquireCommandBuffer(SDL_GPUDevice* device) -> SDL_GPUCommandBuffer* {
  auto* cmd = SDL_AcquireGPUCommandBuffer(device);
  if (cmd == nullptr) {
    panic(std::format("SDL_AcquireGPUCommandBuffer: {}", SDL_GetError()));
  }
  return cmd;
}

//-------------------------------------------------------------------------------------------------
void submit(SDL_GPUCommandBuffer* cmd) {
  if (not SDL_SubmitGPUCommandBuffer(cmd)) {
    panic(std::format("SDL_SubmitGPUCommandBuffer: {}", SDL_GetError()));
  }
}

}  // namespace grape::splat::gpu
