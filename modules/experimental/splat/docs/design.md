# Design

For the theory of gaussian splatting, read [gaussian_splatting.md](./gaussian_splatting.md) first.
This document records the engineering choices made in this viewer.

## Scope

A small, dependency-light viewer for pre-trained 3D gaussian splat scenes. Training, editing and
view-dependent colour are out of scope.

## Scene representation

Each gaussian (`grape::splat::Splat`) has a position, per-axis standard deviation (scale),
orientation (unit quaternion) and an 8-bit RGBA colour, where alpha is the peak opacity. This is
the information content of the `.splat` format ([file_formats.md](./file_formats.md)). The 3D
covariance is `Σ = R·S·Sᵀ·Rᵀ` (Kerbl et al., 2023), computed once on the CPU at load time and
uploaded with the position and colour into a GPU storage buffer (48 bytes per gaussian).

## Rendering pipeline

Per frame:

1. **Sort** (CPU, worker thread): When the camera moves, gaussians in front of the camera are
   ordered back-to-front by view depth. A single pass 16-bit counting sort is used. It is O(N)
   and fast enough to keep up with interaction for ~1M gaussians, and the depth quantisation is
   not visible in practice. Sorting runs asynchronously (`AsyncSorter`) so the render loop never
   stalls; rendering uses the most recent completed order, which may lag the camera by a frame
   or two during fast motion.
2. **Upload**: The order (one `uint32` per visible gaussian) is copied to a GPU storage buffer
   through a cycled transfer buffer, so that the upload does not stall on frames in flight.
3. **Draw**: One instanced draw call with 4 vertices (a triangle strip quad) per visible
   gaussian. The vertex shader (`src/shaders/splat.vert`):
   - Looks up the gaussian through the order buffer
   - Culls gaussians behind the near plane or centred well outside the view
   - Projects the 3D covariance to a 2D screen space covariance with the local affine
     approximation of the perspective projection, `Σ' = J·W·Σ·Wᵀ·Jᵀ` (EWA splatting, Zwicker et
     al., 2001). `W` is the view rotation and `J` the Jacobian of the projection.
   - Adds a 0.3 px² low-pass term so that small gaussians cover at least a pixel
   - Eigen-decomposes the 2D covariance to find the ellipse axes, and places the quad
     corners at ±3σ along each axis
4. **Shade**: The fragment shader (`src/shaders/splat.frag`) evaluates the gaussian falloff
   `α = a·exp(-½·r²)` at the fragment, where `r` is the Mahalanobis distance, and discards
   fragments beyond 3σ or with negligible alpha. Colour is output premultiplied by alpha and
   composited with `dst = src + (1 - src_α)·dst` ('over' operator, back to front).

No depth buffer is used; ordering is entirely by the sort.

Shaders are written in GLSL and compiled to SPIR-V by `glslangValidator` at build time. The
binaries are embedded into the library as generated headers, so the viewer has no runtime file
dependencies. SDL GPU selects the Vulkan backend for SPIR-V.

## Navigation

`ExaminerCamera` reproduces the OpenInventor examiner viewer interaction model. The camera is
described by a focal point, a distance from it, and an orientation. The camera always looks at the
focal point.

- **Rotate**: Virtual trackball (Bell, 1988). Pointer positions are projected onto a sphere
  blended into a hyperbolic sheet, and the rotation between successive projections is applied to
  the scene about the focal point. Releasing the button while moving keeps the scene spinning at
  the last angular velocity (as in OpenInventor).
- **Pan**: Translate camera and focal point in the view plane, scaled so that points at the focal
  distance follow the pointer exactly.
- **Dolly**: Scale the distance to the focal point exponentially with pointer motion, so the
  response feels the same at any scale.
- **Seek**: Pick the nearest sufficiently opaque gaussian under the pointer (CPU ray proximity
  test) and make it the new focal point, moving half way towards it.
- **View all**: Frame the scene's main subject. Captured scenes contain a sparse shell of
  distant background gaussians, so the bounding sphere is centred at the per-axis median of
  positions with radius at the median distance from it (`computeBounds()`), rather than the
  conventional enclosing sphere.

## Coordinate conventions

Camera frame follows OpenGL/OpenInventor: x right, y up, looking along -z. Scene files carry no
up-axis information; most captured scenes (3DGS training output) use the COLMAP/OpenCV RDF
convention (y down), which is why the viewer's default up axis is `-y`.

## References

- B. Kerbl, G. Kopanas, T. Leimkühler, G. Drettakis, "3D Gaussian Splatting for Real-Time
  Radiance Field Rendering", ACM Trans. Graphics (SIGGRAPH), 2023.
  <https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/>
- M. Zwicker, H. Pfister, J. van Baar, M. Gross, "EWA Splatting", IEEE TVCG, 2002.
- G. Bell, "Virtual trackball", SGI, 1988 (as used in OpenInventor and GLUT examples).
- antimatter15/splat, WebGL gaussian splat viewer: <https://github.com/antimatter15/splat>
- J. Wernecke, "The Inventor Mentor", Addison-Wesley, 1994 (examiner viewer behaviour).
