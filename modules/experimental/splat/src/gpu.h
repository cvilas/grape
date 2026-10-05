//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

// Thin RAII wrappers over the SDL3 GPU API.
//
// This is plumbing: none of it is specific to gaussian splatting. It is kept out of viewer.cpp so
// that the renderer reads as a description of the algorithm. Every function panics on failure.

#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "grape/utils/attributes.h"

namespace grape::splat::gpu {

//-------------------------------------------------------------------------------------------------
/// Initialises the SDL video subsystem for the lifetime of the object
struct SdlVideo {
  SdlVideo();
  ~SdlVideo();
  SdlVideo(const SdlVideo&) = delete;
  SdlVideo(SdlVideo&&) = delete;
  auto operator=(const SdlVideo&) = delete;
  auto operator=(SdlVideo&&) = delete;
};

//-------------------------------------------------------------------------------------------------
/// Owns a resource created from a GPU device
template <typename T, void (*Release)(SDL_GPUDevice*, T*)>
class Handle {
public:
  Handle() = default;
  Handle(SDL_GPUDevice* device GRAPE_LIFETIMEBOUND, T* handle GRAPE_LIFETIMEBOUND)
    : device_(device), handle_(handle) {
  }
  ~Handle() {
    reset();
  }
  Handle(Handle&& other) noexcept
    : device_(std::exchange(other.device_, nullptr))
    , handle_(std::exchange(other.handle_, nullptr)) {
  }
  auto operator=(Handle&& other) noexcept -> Handle& {
    if (this != &other) {
      reset();
      device_ = std::exchange(other.device_, nullptr);
      handle_ = std::exchange(other.handle_, nullptr);
    }
    return *this;
  }
  Handle(const Handle&) = delete;
  auto operator=(const Handle&) = delete;

  void reset() {
    if (handle_ != nullptr) {
      Release(device_, handle_);
    }
    handle_ = nullptr;
  }

  [[nodiscard]] auto get() const -> T* {
    return handle_;
  }

private:
  SDL_GPUDevice* device_{ nullptr };
  T* handle_{ nullptr };
};

using Shader = Handle<SDL_GPUShader, SDL_ReleaseGPUShader>;
using Pipeline = Handle<SDL_GPUGraphicsPipeline, SDL_ReleaseGPUGraphicsPipeline>;
using Buffer = Handle<SDL_GPUBuffer, SDL_ReleaseGPUBuffer>;
using TransferBuffer = Handle<SDL_GPUTransferBuffer, SDL_ReleaseGPUTransferBuffer>;

//-------------------------------------------------------------------------------------------------
/// Binds a window to a GPU device for presentation, for the lifetime of the object
class WindowClaim {
public:
  WindowClaim(SDL_GPUDevice* device GRAPE_LIFETIMEBOUND, SDL_Window* window GRAPE_LIFETIMEBOUND);
  ~WindowClaim();
  WindowClaim(const WindowClaim&) = delete;
  WindowClaim(WindowClaim&&) = delete;
  auto operator=(const WindowClaim&) = delete;
  auto operator=(WindowClaim&&) = delete;

private:
  SDL_GPUDevice* device_;
  SDL_Window* window_;
};

/// Create a shader from a SPIR-V binary
auto createShader(SDL_GPUDevice* device GRAPE_LIFETIMEBOUND, std::span<const std::uint32_t> spirv,
                  SDL_GPUShaderStage stage, std::uint32_t num_storage_buffers,
                  std::uint32_t num_uniform_buffers) -> Shader;

/// Create a GPU buffer that shaders can read as a storage buffer
auto createStorageBuffer(SDL_GPUDevice* device GRAPE_LIFETIMEBOUND, std::size_t size_bytes)
    -> Buffer;

/// Create a CPU-writable staging buffer for uploads to GPU buffers
auto createTransferBuffer(SDL_GPUDevice* device GRAPE_LIFETIMEBOUND, std::size_t size_bytes)
    -> TransferBuffer;

/// Copy bytes into a transfer buffer, and record an upload from it into a GPU buffer
void upload(SDL_GPUDevice* device, SDL_GPUCommandBuffer* cmd, SDL_GPUTransferBuffer* transfer,
            SDL_GPUBuffer* buffer, std::span<const std::byte> bytes);

/// Acquire a command buffer
auto acquireCommandBuffer(SDL_GPUDevice* device) -> SDL_GPUCommandBuffer*;

/// Submit a command buffer
void submit(SDL_GPUCommandBuffer* cmd);

}  // namespace grape::splat::gpu
