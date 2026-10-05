# Splat

Interactive viewer for 3D gaussian splat scenes, rendered with the SDL3 GPU API.

![Demo](./docs/demo.png)

New to gaussian splatting? Start with the primer, [docs/gaussian_splatting.md](./docs/gaussian_splatting.md).
It explains the ideas from first principles and maps each one to the code.

## Usage

```bash
grape_splat_view --file=modules/experimental/splat/models/hornedlizard.splat
```

Option   | Default | Description
---------|---------|------------
`--file` | -       | Scene file (`.splat` or `.ply`)
`--up`   | `-y`    | World axis shown as screen-up (`x`, `-x`, `y`, `-y`, `z`, `-z`). Most captured scenes use y-down (RDF) coordinates
`--fov`  | `50`    | Vertical field of view (degrees)

See [models/README.md](./models/README.md) for sample scenes, and `examples/example.cpp` for 
viewing procedurally generated scenes.

## Controls

Navigation follows the [OpenInventor](https://github.com/coin3d/coin) examiner viewer. The camera
orbits a focal point at the centre of the view.

Input                                                           | Action
----------------------------------------------------------------|--------
Left drag                                                       | Rotate about the focal point (virtual trackball)
Release left button while moving                                | Keep spinning. Click to stop.
Middle drag, or Ctrl/Shift + left drag                          | Pan
Right drag, left + middle drag, Ctrl + middle drag, or Ctrl + Shift + left drag | Dolly (drag up to move closer)
Scroll wheel                                                    | Dolly
`S`, then left click on the scene                               | Seek: re-centre on the clicked point, and move half way towards it
`Esc`                                                           | Cancel seek
`V`                                                             | View all
`H`                                                             | Go to home view
`Shift` + `H`                                                   | Set home view to the current view

## File formats

- `.splat` (native): The compact format from [antimatter15/splat](https://github.com/antimatter15/splat), 
  widely supported by web viewers and conversion tools. 32 bytes per gaussian, colour without 
  view-dependent effects. Chosen for being simple, compact and memory-mappable.
- `.ply`: Output of the reference [3D gaussian splatting](https://github.com/graphdeco-inria/gaussian-splatting) 
  implementation (binary little-endian). Only the view-independent (DC) colour is used.

Layouts are described in [docs/file_formats.md](./docs/file_formats.md). Use 
`scripts/spz_to_splat.py` to convert Niantic `.spz` files.

## Design

- [docs/gaussian_splatting.md](./docs/gaussian_splatting.md): How gaussian splatting works
- [docs/design.md](./docs/design.md): Engineering choices made in this viewer

Source layout:

File                                  | Contents
--------------------------------------|---------
`include/grape/splat/splat.h`         | The splat primitive and its covariance
`src/shaders/splat.vert`, `splat.frag`| Projection and shading of one splat. The heart of the renderer
`include/grape/splat/depth_sort.h`    | Back to front ordering
`src/viewer.cpp`                      | Frame loop, GPU pipeline, mouse and keyboard handling
`include/grape/splat/examiner_camera.h` | OpenInventor-style orbiting camera
`include/grape/splat/file_io.h`       | `.splat` and `.ply` reading and writing
`include/grape/splat/math.h`          | Minimal vector, quaternion and matrix types
`src/gpu.h`, `src/async_sorter.h`     | SDL GPU and threading plumbing

## Limitations

- Requires a Vulkan capable GPU (shaders are compiled to SPIR-V at build time)
- No view-dependent colour (spherical harmonics beyond degree 0 are ignored)
- Depth sort is per-splat on the CPU. Overlapping splats may 'pop' as the view changes.
- Build requires `glslangValidator` (`glslang-tools` package, installed by 
  `toolchains/install_base.sh`)

## Testing without a GPU

The viewer runs on Mesa's software Vulkan driver (lavapipe), for example on a headless machine:

```bash
sudo apt install mesa-vulkan-drivers xvfb
Xvfb :99 & DISPLAY=:99 grape_splat_view --file=...
```
