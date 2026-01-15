# Graphics

## SDL vs GLFW

- GLFW is much simpler and mostly sufficient for our needs
- GLFW is required for MuJoCo

However, SDL3 offers the following advantages
- Platform independent camera support
- Platform independent 3D graphics and GPU compute support.
  - Easiest way to exploit Vulkan/Metal/Direct3D
- Platform independent hardware accelerated 2D graphics support
- Text support with SDL_ttf
- Images support with SDL_image