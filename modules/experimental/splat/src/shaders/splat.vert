//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

// Splat vertex shader: turns one 3D gaussian into a screen-space quad that covers its 2D footprint.
//
// Runs 4 times per splat (one per quad corner); the instance index selects the splat. The theory
// behind each step is in docs/gaussian_splatting.md, section "Rendering".
//
//   1. Transform the splat centre into camera space, and cull it if it is behind the camera.
//   2. Project the centre to the screen.
//   3. Project the 3D covariance Σ to a 2D screen-space covariance Σ' = J·W·Σ·Wᵀ·Jᵀ.
//   4. Find the axes of the ellipse described by Σ' (eigen-decomposition).
//   5. Place this vertex at one corner of a rectangle that spans ±3σ along those axes.
//
// Resource bindings follow SDL GPU conventions for SPIR-V vertex shaders:
// set 0: storage buffers, set 1: uniform buffers

#version 450

// One splat. Layout must match GpuSplat in viewer.cpp
struct GpuSplat {
  vec3 position;  // world frame
  uint rgba;      // packed RGBA8, R in least significant byte
  vec3 cov_a;     // 3D covariance Σ (symmetric): xx, xy, xz
  float pad0;
  vec3 cov_b;     // yy, yz, zz
  float pad1;
};

layout(std430, set = 0, binding = 0) readonly buffer SplatBuffer {
  GpuSplat splats[];
};

// Splat indices sorted far to near
layout(std430, set = 0, binding = 1) readonly buffer OrderBuffer {
  uint order[];
};

layout(std140, set = 1, binding = 0) uniform CameraUniforms {
  mat4 view;      // world to camera. Camera looks along -Z, +Y up, +X right
  vec2 focal;     // focal length (pixels)
  vec2 viewport;  // viewport size (pixels)
};

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_offset;  // this corner, in standard deviations along each axis

// A gaussian never reaches zero, so it must be cut off somewhere. Beyond 3σ, the falloff
// exp(-½·3²) ≈ 1% is invisible.
const float EXTENT = 3.0;

const float NEAR = 0.01;  // near clip distance (m)

// Splats centred just outside the view can still reach into it. Keep those within this multiple
// of the half-viewport.
const float FRUSTUM_MARGIN = 1.3;

// Limit on the projected size (in standard deviations, pixels) of a splat very close to the camera
const float MAX_SIGMA_PX = 1024.0;

// Variance (px²) added to every projected gaussian. A pixel samples the image at a single point;
// a gaussian much smaller than a pixel would flicker in and out of existence as it moves. Blurring
// it to at least about a pixel wide is a cheap anti-aliasing filter (Kerbl et al. use the same 0.3)
const float LOW_PASS_PX2 = 0.3;

// A vertex position that the GPU will clip, which discards the whole quad
const vec4 CULLED = vec4(0.0, 0.0, 2.0, 1.0);

// Quad corners in triangle strip order
const vec2 CORNERS[4] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));

void main() {
  const GpuSplat s = splats[order[gl_InstanceIndex]];

  //-----------------------------------------------------------------------------------------------
  // 1. Centre in camera space. t.z is negative in front of the camera.
  const vec3 t = (view * vec4(s.position, 1.0)).xyz;
  const float depth = -t.z;
  if (depth < NEAR) {
    gl_Position = CULLED;
    return;
  }

  //-----------------------------------------------------------------------------------------------
  // 2. Pinhole projection: (u, v) = focal·(x, y)/depth, in pixels from the viewport centre (+y up)
  const vec2 half_viewport = 0.5 * viewport;
  const vec2 center_px = focal * t.xy / depth;
  if (any(greaterThan(abs(center_px), FRUSTUM_MARGIN * half_viewport))) {
    gl_Position = CULLED;
    return;
  }

  //-----------------------------------------------------------------------------------------------
  // 3. Project the covariance.
  //
  // A linear map M transforms a gaussian with covariance Σ into another gaussian with covariance
  // M·Σ·Mᵀ. The world-to-camera rotation W is linear, so Σ_camera = W·Σ·Wᵀ exactly.
  //
  // Perspective projection is not linear (it divides by depth), so a projected gaussian is not
  // exactly a gaussian. EWA splatting (Zwicker et al., 2001) approximates the projection near the
  // splat centre by its first-order Taylor expansion, i.e. its Jacobian J:
  //
  //        | ∂u/∂x  ∂u/∂y  ∂u/∂z |   | f/d   0    f·x/d² |
  //    J = |                     | = |                   |   (d = depth = -z)
  //        | ∂v/∂x  ∂v/∂y  ∂v/∂z |   |  0   f/d   f·y/d² |
  //
  // Splats are small, so the approximation is good. Its third row is unused (zero) here.
  //
  // At the edges of a wide view, x/d grows large and J distorts splats badly. Like the reference
  // implementation, evaluate J with x/d and y/d clamped to slightly outside the view.
  const vec2 lim = FRUSTUM_MARGIN * half_viewport / focal;
  const vec2 txy = clamp(t.xy / depth, -lim, lim) * depth;
  const float inv_d = 1.0 / depth;
  const float inv_d2 = inv_d * inv_d;
  // GLSL matrices are built column by column
  const mat3 J = mat3(focal.x * inv_d, 0.0, 0.0,                           // column 0: ∂/∂x
                      0.0, focal.y * inv_d, 0.0,                           // column 1: ∂/∂y
                      focal.x * txy.x * inv_d2, focal.y * txy.y * inv_d2, 0.0);  // column 2: ∂/∂z
  const mat3 W = mat3(view);  // rotation part of the view transform
  const mat3 T = J * W;
  const mat3 sigma = mat3(s.cov_a.x, s.cov_a.y, s.cov_a.z,
                          s.cov_a.y, s.cov_b.x, s.cov_b.y,
                          s.cov_a.z, s.cov_b.y, s.cov_b.z);
  const mat3 cov = T * sigma * transpose(T);  // top-left 2x2 is the screen-space covariance Σ'

  //-----------------------------------------------------------------------------------------------
  // 4. Ellipse axes. The 2D covariance Σ' = [a b; b c] describes an ellipse whose axes point along
  // the eigenvectors of Σ', with standard deviations equal to the square roots of the eigenvalues.
  // For a symmetric 2x2 matrix, the eigenvalues have the closed form mid ± radius below.
  const float a = cov[0][0] + LOW_PASS_PX2;
  const float b = cov[0][1];
  const float c = cov[1][1] + LOW_PASS_PX2;
  const float mid = 0.5 * (a + c);
  const float radius = length(vec2(0.5 * (a - c), b));
  const float lambda1 = mid + radius;               // major axis variance
  const float lambda2 = max(mid - radius, 0.0);     // minor axis variance
  // Eigenvector of lambda1. If b = 0 the matrix is already diagonal and the axes are x and y.
  const vec2 e1 = (abs(b) < 1e-9) ? ((a >= c) ? vec2(1.0, 0.0) : vec2(0.0, 1.0))
                                   : normalize(vec2(b, lambda1 - a));
  const vec2 e2 = vec2(-e1.y, e1.x);  // perpendicular
  const vec2 axis1 = min(sqrt(lambda1), MAX_SIGMA_PX) * e1;  // 1σ along the major axis (pixels)
  const vec2 axis2 = min(sqrt(lambda2), MAX_SIGMA_PX) * e2;  // 1σ along the minor axis (pixels)

  //-----------------------------------------------------------------------------------------------
  // 5. Emit this corner of the ±3σ rectangle, aligned with the ellipse axes.
  // Pixel coordinates are converted to normalised device coordinates [-1, 1]. Depth is a constant
  // because draw order alone decides visibility.
  const vec2 corner = EXTENT * CORNERS[gl_VertexIndex];
  const vec2 pos_px = center_px + corner.x * axis1 + corner.y * axis2;
  gl_Position = vec4(pos_px / half_viewport, 0.5, 1.0);

  // The corner, measured in standard deviations along the ellipse axes. The rasteriser
  // interpolates it linearly across the quad, which gives the fragment shader each pixel's
  // position in the splat's own 'unit circle' coordinates for free.
  v_offset = corner;
  v_color = unpackUnorm4x8(s.rgba);
}
