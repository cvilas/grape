# Gaussian splatting: a primer

This primer explains 3D gaussian splatting from first principles. It assumes linear algebra and
basic computer graphics (cameras, projection, alpha blending), and nothing about splats. Each
section links to the code that implements it, so you can read the two together.

Suggested reading order:

1. This document, sections 1 to 5 (what a splat is)
2. [`splat.h`](../include/grape/splat/splat.h) and [`splat.cpp`](../src/splat.cpp)
3. Section 6 (rendering), alongside [`splat.vert`](../src/shaders/splat.vert),
   [`splat.frag`](../src/shaders/splat.frag), [`depth_sort.cpp`](../src/depth_sort.cpp) and the
   top of [`viewer.cpp`](../src/viewer.cpp)
4. Sections 7 and 8 (where scenes come from, file formats), with
   [`file_io.cpp`](../src/file_io.cpp)
5. Section 10 (experiments to try)

---

## 1. The big idea

There are several ways to represent a 3D scene for rendering:

- **Triangle meshes**: surfaces made of flat triangles. Compact and fast, but hard to produce
  automatically from photos of real scenes, especially for hair, foliage and fuzzy or
  semi-transparent things.
- **Point clouds**: coloured points, as produced by 3D scanners. Easy to capture, but points have
  no size, so the rendered image is full of holes.
- **Neural radiance fields (NeRF)**: a neural network that returns colour and density at any point
  in space. Photorealistic, but rendering a pixel means evaluating the network hundreds of times,
  so it is slow.

**Gaussian splatting** (Kerbl et al., 2023) sits between point clouds and NeRF. The scene is a cloud
of millions of small, soft, coloured, semi-transparent **ellipsoids**. Each one is a 3D gaussian
'blob'. They overlap and blend together into continuous surfaces. Rendering is simple and fast:
project each blob onto the screen as a soft ellipse, and blend the ellipses back to front. This
runs at hundreds of frames per second on a GPU, at a quality comparable to NeRF.

'Splatting' is an old term (Westover, 1990): rendering by throwing each primitive onto the screen,
like a snowball against a wall, rather than tracing rays through the scene.

The blobs are not modelled by hand. Their positions, shapes, colours and opacities are found by
**optimisation**: starting from a rough point cloud, adjust the parameters until rendered images
match a set of photographs (section 7). This viewer only renders the result.

## 2. The gaussian, from 1D to 3D

### 1D

The gaussian (normal distribution, bell curve) centred at μ with standard deviation σ is:

```text
G(x) = exp(-½·(x - μ)²/σ²)
```

It equals 1 at the centre, and decays smoothly and symmetrically, never reaching zero. Some values
worth remembering:

Distance from centre | G
---------------------|------
1σ                   | 0.61
2σ                   | 0.14
3σ                   | 0.011

Beyond 3σ, the curve is negligible. Renderers therefore cut it off there (`EXTENT` in
[`splat.vert`](../src/shaders/splat.vert)).

(Statistics normalises G so that it integrates to 1. Splatting does not; the peak is always 1 and is
then scaled by an opacity.)

### 3D

In 3D, σ² is replaced by a 3x3 **covariance matrix** Σ:

```text
G(p) = exp(-½·(p - μ)ᵀ·Σ⁻¹·(p - μ))
```

The quantity `r = sqrt((p - μ)ᵀ·Σ⁻¹·(p - μ))` is the **Mahalanobis distance**: the distance from the
centre measured in standard deviations, which may differ along each direction. Surfaces of constant
r are ellipsoids. The 3σ ellipsoid (r = 3) contains nearly all of the blob.

- The diagonal elements Σxx, Σyy, Σzz are the variances (σ²) along x, y and z
- The off-diagonal elements describe how the ellipsoid is tilted relative to the axes
- Σ is symmetric, so only 6 of its 9 numbers are independent
  ([`Covariance`](../include/grape/splat/splat.h))
- Σ must be positive semi-definite (no negative variances in any direction)

The eigenvectors of Σ are the directions of the ellipsoid's axes, and the square roots of its
eigenvalues are the standard deviations along them. This is used in reverse in the next section,
and again in 2D in section 6.

## 3. Describing the shape: scale and rotation

An optimiser that adjusts the 6 numbers of Σ directly could easily produce an invalid
(non-positive-definite) matrix. Instead, each splat stores the intuitive quantities:

- **scale** s = (sx, sy, sz): the standard deviation along each of the ellipsoid's own axes
- **rotation** q: a unit quaternion that orients those axes in the world

and computes:

```text
Σ = R·S·Sᵀ·Rᵀ        where R = rotation matrix of q, S = diag(sx, sy, sz)
```

Read right to left: take a unit sphere, stretch it by S along x, y and z, then rotate it by R. Any
matrix of the form M·Mᵀ is symmetric and positive semi-definite, so Σ is always valid. This is
[`covariance()`](../src/splat.cpp).

**Worked example.** A needle 1 m long (σ) in x and 0.1 m thick, turned 90° about z:

```text
S = diag(1, 0.1, 0.1),   R = rotation of 90° about z (maps x → y)
M = R·S has columns R·(1,0,0)·1 = (0,1,0), R·(0,1,0)·0.1 = (-0.1,0,0), R·(0,0,1)·0.1 = (0,0,0.1)
Σ = M·Mᵀ = diag(0.01, 1, 0.01)
```

The needle now points along y, as expected.

## 4. What a splat stores

Parameter       | Count | Meaning
----------------|-------|--------
position μ      | 3     | Centre
scale           | 3     | Standard deviations along the local axes
rotation        | 4     | Unit quaternion
opacity         | 1     | Peak opacity α at the centre, in (0, 1)
colour (SH)     | 48    | Colour as a function of view direction (next section). 3 if view independent

That is 59 numbers per splat in a full scene, or 14 without view-dependent colour. A scene has from
a few hundred thousand to a few million splats, so files are tens to hundreds of megabytes.

This viewer stores the 14-number form, quantised into the 32 bytes of a `.splat` record
([`Splat`](../include/grape/splat/splat.h), [file_formats.md](./file_formats.md)).

## 5. Colour and spherical harmonics

Real surfaces can look different from different directions (reflections, sheen). To capture this,
each splat's colour is a function of the viewing direction, expressed as a sum of **spherical
harmonics** (SH): basis functions on the sphere, analogous to a Fourier series on a circle.

- Degree 0: 1 basis function, a constant. This is the plain, view-independent colour.
- Degrees 0 to 3: 1 + 3 + 5 + 7 = 16 basis functions, × 3 colour channels = 48 coefficients.

The degree 0 basis function is the constant `C0 = 1/(2·sqrt(π)) ≈ 0.282`. Training stores the colour
as a coefficient f_dc such that `colour = 0.5 + C0·f_dc` (the 0.5 makes a zero coefficient mid-grey).

This viewer uses only degree 0. Diffuse scenes look nearly the same; shiny surfaces look flatter.

## 6. Rendering

The renderer turns a list of 3D blobs into an image in four steps. Overview in
[`viewer.cpp`](../src/viewer.cpp).

```text
            scene (fixed)                 every time the camera moves
splats ──► 3D covariances ──► GPU    camera ──► sort far to near ──► GPU
                                                                      │
             for each splat, in order: project ─► quad ─► shade ─► blend ─► image
```

### 6.1 Projecting a gaussian to the screen

A useful property of gaussians: **a linear transformation of a gaussian is another gaussian.** If
points are mapped by p' = M·p, a gaussian with covariance Σ becomes one with covariance M·Σ·Mᵀ.

The world-to-camera transform is a rotation W plus a translation. Translation moves the centre and
leaves the shape alone, so in camera space:

```text
Σ_camera = W·Σ·Wᵀ
```

Perspective projection, (x, y, z) → (u, v) = f·(x, y)/d with d the depth, is not linear: it divides
by depth. A projected gaussian is not exactly a gaussian. The fix (Zwicker et al., 2001, 'EWA
splatting') is to approximate the projection near the splat's centre by a linear map: its first
order Taylor expansion, the **Jacobian** J of (u, v) with respect to (x, y, z):

```text
J = | f/d   0    f·x/d² |        evaluated at the splat centre (x, y, d)
    |  0   f/d   f·y/d² |
```

The first two columns say that lateral sizes shrink as 1/d. The third says that moving away from the
camera also shifts a point towards the image centre. Splats are small compared to their distance,
so the approximation is good. Then the screen-space 2x2 covariance is:

```text
Σ' = J·W·Σ·Wᵀ·Jᵀ
```

For example, a round splat with σ = 0.1 m at d = 2 m on the optical axis, with f = 1000 px, has a
screen-space σ' = f·σ/d = 50 px.

**Anti-aliasing.** A splat much smaller than a pixel can fall between pixel centres and vanish, then
reappear as it moves, causing flicker. Adding a small constant (0.3 px²) to the diagonal of Σ' blurs
every splat to at least about a pixel wide. This is `LOW_PASS_PX2`.

Code: steps 1 to 3 in [`splat.vert`](../src/shaders/splat.vert).

### 6.2 Covering the ellipse with a quad

The GPU draws triangles, not ellipses. For each splat, the vertex shader emits a rectangle (two
triangles) that just covers the 3σ ellipse:

1. Eigen-decompose Σ' = [a b; b c]. The eigenvalues λ1 ≥ λ2 have the closed form
   `(a + c)/2 ± sqrt(((a - c)/2)² + b²)`. The eigenvectors e1, e2 give the ellipse axes, with
   standard deviations sqrt(λ1) and sqrt(λ2).
2. Place the corners at `centre ± 3·sqrt(λ1)·e1 ± 3·sqrt(λ2)·e2`.

Each corner also passes its position in units of σ along the axes, (±3, ±3), to the fragment
shader. The GPU interpolates this linearly across the quad, so every pixel receives its position
relative to the splat in a frame where the ellipse is a unit circle.

Code: steps 4 and 5 in [`splat.vert`](../src/shaders/splat.vert).

### 6.3 Shading a pixel

In those coordinates, the Mahalanobis distance is just the length r of the interpolated vector, and
the splat's opacity at that pixel is:

```text
α = opacity · exp(-½·r²)
```

Pixels beyond r = 3 (the corners of the quad), or with negligible α, are discarded.

Code: [`splat.frag`](../src/shaders/splat.frag).

### 6.4 Blending

Each pixel is covered by many overlapping, semi-transparent splats. The final colour is (Porter and
Duff, 1984), with splats numbered from nearest (1) to farthest (N):

```text
C = Σᵢ cᵢ·αᵢ·Tᵢ        where Tᵢ = (1 - α₁)·(1 - α₂)···(1 - αᵢ₋₁)
```

Tᵢ is the **transmittance**: the fraction of light from splat i that gets through all the splats in
front of it. This is the same formula NeRF uses to render a ray through a volume; gaussian
splatting evaluates it with blobs instead of samples along a ray.

The GPU computes it without any explicit sum, by drawing splats **from back to front**, and
blending each over the image so far:

```text
image ← cᵢ·αᵢ + (1 - αᵢ)·image
```

Expanding this recurrence from i = N down to 1 gives exactly the sum above. The fragment shader
outputs `(c·α, α)` ('premultiplied alpha'), and the blend state in `createPipeline()`
([`viewer.cpp`](../src/viewer.cpp)) does the rest.

No depth buffer is needed or used: draw order alone determines what is in front.

### 6.5 Sorting

Back to front blending needs the splats sorted by distance from the camera, which changes whenever
the camera moves. Two simplifications make this fast:

- **Sort per splat, not per pixel.** Each splat gets a single depth, that of its centre. Where two
  splats intersect, the correct order differs across the pixels they share, so the result is
  occasionally wrong. This shows as brief 'popping' when the order flips as the view changes. It is
  rarely noticeable because splats are small.
- **Sort approximately.** Depths are quantised into 65536 buckets and sorted with a counting sort,
  which is linear in the number of splats.

The sort runs on a background thread, and rendering continues with the previous order until a new
one is ready.

Code: [`depth_sort.cpp`](../src/depth_sort.cpp), [`async_sorter.h`](../src/async_sorter.h).

## 7. Where scenes come from

This viewer does not create scenes, but knowing how they are made explains their contents.

1. **Capture**: Take 50 to a few hundred photos of a scene from many viewpoints.
2. **Structure from motion** (e.g. COLMAP): Estimate each photo's camera pose, and a sparse point
   cloud of recognisable features.
3. **Initialise** one small, round gaussian per point.
4. **Optimise** (about 30,000 iterations of gradient descent): Render the splats from a training
   photo's viewpoint, compare with the photo, and adjust every splat's parameters to reduce the
   difference. The renderer is differentiable, which makes this possible.
5. **Densify and prune**: Periodically split large splats, clone small ones in regions that are
   poorly reconstructed, and delete nearly transparent ones.

Consequences that are visible in this viewer:

- **Stored values are unconstrained.** Gradient descent works best on parameters with no limits. So
  scale is stored as log(σ) and opacity as a logit, and mapped back with exp() and the sigmoid
  function when loaded (`decodePly()` in [`file_io.cpp`](../src/file_io.cpp)).
- **Background clutter.** Splats are created wherever they help to match the photos, including far
  away sky and walls. 'View all' frames the median of splat positions instead of their full
  extent (`computeBounds()` in [`splat.cpp`](../src/splat.cpp)).
- **Novel views degrade.** Quality is best near the capture viewpoints. Look from directions where
  there were no photos, and needle-like or misplaced splats ('floaters') become visible.
- **Coordinate frame.** COLMAP uses camera-style coordinates with y pointing down, so most scenes
  appear upside down unless the viewer uses `-y` as up (the default here).

## 8. File formats

There is no single standard. The two formats this viewer reads are described in
[file_formats.md](./file_formats.md):

- **`.ply`**: The direct output of training. Every parameter as a 32-bit float, including all 48 SH
  coefficients. Large, lossless.
- **`.splat`**: A compact 32-byte record per splat with view-independent colour, opacity and
  rotation quantised to 8 bits.

## 9. Concepts to code

Concept                                   | Code
------------------------------------------|-----
Splat parameters                          | `Splat` in [`splat.h`](../include/grape/splat/splat.h)
Σ = R·S·Sᵀ·Rᵀ                             | `covariance()` in [`splat.cpp`](../src/splat.cpp)
Quaternion to rotation matrix             | `toMatrix()` in [`math.h`](../include/grape/splat/math.h)
Projection Σ' = J·W·Σ·Wᵀ·Jᵀ               | Step 3 in [`splat.vert`](../src/shaders/splat.vert)
Ellipse axes, quad                        | Steps 4 and 5 in [`splat.vert`](../src/shaders/splat.vert)
Gaussian falloff α = o·exp(-½r²)          | [`splat.frag`](../src/shaders/splat.frag)
Back to front blending                    | `createPipeline()` in [`viewer.cpp`](../src/viewer.cpp)
Depth sort                                | `sortBackToFront()` in [`depth_sort.cpp`](../src/depth_sort.cpp)
Frame loop                                | `Viewer::Impl::render()` in [`viewer.cpp`](../src/viewer.cpp)
Log scale, logit opacity, SH colour       | `decodePly()` in [`file_io.cpp`](../src/file_io.cpp)
Camera and navigation                     | [`examiner_camera.h`](../include/grape/splat/examiner_camera.h), [design.md](./design.md)

## 10. Experiments

Change one thing, rebuild, and look at a scene. Each one isolates an idea from above.

1. **See the blobs.** In `splat.frag`, replace `exp(-0.5 * r2)` with `1.0` (and `MAX_ALPHA` with
   1.0). Every splat becomes a hard-edged opaque ellipse, and the structure of the scene is laid
   bare.
2. **Shrink every splat.** In `toGpuSplat()` ([`viewer.cpp`](../src/viewer.cpp)), scale the
   covariance by 0.25 (σ halved). The scene dissolves into separate dots: overlap is what makes
   surfaces continuous.
3. **Skip the sort.** In `sortBackToFront()`, return the visible indices in file order. Blending
   now composites in the wrong order and surfaces become murky and view dependent (section 6.4).
4. **Cut off early.** Set `EXTENT` to 1.0 in `splat.vert` and `EXTENT_SQ` to 1.0 in `splat.frag`.
   Each splat is truncated at 1σ (where G is still 0.61) and the scene looks blotchy.
5. **No anti-aliasing.** Set `LOW_PASS_PX2` to 0 and dolly away from a scene. Fine detail
   shimmers as you move.
6. **Write your own scene.** [`examples/example.cpp`](../examples/example.cpp) builds a scene in
   code. Try a single splat and check that its on-screen size matches section 6.1.

## 11. Glossary

Term              | Meaning
------------------|--------
Splat             | One 3D gaussian primitive; also the act of projecting it to the screen
Covariance Σ      | 3x3 symmetric matrix describing the size, shape and orientation of a gaussian
Mahalanobis distance | Distance from the centre in units of standard deviation
EWA               | Elliptical weighted average: projecting gaussians with a local linear approximation
Jacobian J        | Matrix of partial derivatives; the best linear approximation of a function at a point
Opacity α         | Fraction of light blocked. 0 is transparent, 1 is opaque
Transmittance T   | Fraction of light that passes through everything in front
Premultiplied alpha | Storing colour as (c·α, α), which makes blending a single multiply-add
SH                | Spherical harmonics: basis functions for view-dependent colour
DC                | The degree 0 (constant) SH term, by analogy with 'direct current'
SfM               | Structure from motion: recovering camera poses and sparse 3D points from photos
Floater           | A misplaced splat, visible from viewpoints far from the training photos

## 12. Further reading

- B. Kerbl, G. Kopanas, T. Leimkühler, G. Drettakis, "3D Gaussian Splatting for Real-Time Radiance
  Field Rendering", ACM Trans. Graphics (SIGGRAPH), 2023.
  <https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/>. The paper. Sections 4 and 6 cover
  what this viewer does.
- M. Zwicker, H. Pfister, J. van Baar, M. Gross, "EWA Splatting", IEEE TVCG, 2002. Projection of
  gaussians, section 6.1.
- L. Westover, "Footprint Evaluation for Volume Rendering", SIGGRAPH, 1990. Origin of 'splatting'.
- T. Porter, T. Duff, "Compositing Digital Images", SIGGRAPH, 1984. The 'over' operator.
- B. Mildenhall et al., "NeRF: Representing Scenes as Neural Radiance Fields for View Synthesis",
  ECCV, 2020. The volume rendering formula of section 6.4.
- antimatter15/splat, a compact WebGL viewer and the origin of the `.splat` format:
  <https://github.com/antimatter15/splat>
