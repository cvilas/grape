# Design notes: Linear algebra library

## 1. The big idea

Most geometry bugs in robotics are not arithmetic errors. They are *bookkeeping* errors: a vector
expressed in the camera frame added to one expressed in the world frame, a rotation applied in the
wrong direction, a position treated as if it were a direction. The arithmetic is perfectly happy to
multiply the numbers; the result is silently wrong, and the robot drives into a wall. The loss of
the Mars Climate Orbiter (1999) was exactly this class of error, in unit rather than frame
bookkeeping.

The numbers cannot catch these errors, but the *types* can. This module is built in two layers:

- **A numeric kernel**: plain matrices, vectors and quaternions templated over their dimensions. 
  Operations between incompatible types (eg: 3x4 matrix times 2x2 matrix) are compile-time errors.
- **A frame-aware layer**: wrappers that add *which frame, which points* to each quantity as 
  zero-cost type tags. Mixing incompatible frames are compile-time errors.

The conventions follow [P. Furgale, *Representing Robot Pose: The Good, the Bad, and the Ugly*](./furgale.pdf).

## 2. Matrices from first principles

Matrix multiplication (`operator*`) iterates in `i-k-j` order rather than the textbook `i-j-k`.
Both compute the same sum; the difference is memory access. With `j` innermost, both `result[i, j]`
and `rhs[k, j]` walk along a row — consecutive memory — while the textbook order makes `rhs[k, j]`
jump a whole row ahead on every step. For the small matrices here either is fast; the habit matters
when the same code grows blocks, tiles, and SIMD (see the 'Stellar' lecture in section 9).

## 3. Numerical care

**Fused multiply-add** (`detail::multiplyAdd()`). Hardware FMA computes `a*b + c` with a *single*
rounding instead of two, halving the error of every accumulation and running at least as fast.
Every dot product, matrix product and quaternion product in this module accumulates through it. In
a constant expression it falls back to `a*b + c` — at compile time there is no hardware to fuse,
and the operations must stay evaluable.

**Kahan's difference of products** (`differenceOfProducts()`). The expression `a*b - c*d` is
treacherous when the two products nearly cancel: each product is rounded *before* the subtraction,
and the subtraction then amplifies those roundings into the answer ('catastrophic cancellation').
The cross product is made of exactly such expressions. Kahan's trick recovers the rounding error of
one product with an FMA (`err = fma(-c, d, c*d)` is *exactly* the error, by construction) and adds
it back, making the result accurate to the last bit or two. See the pbrt blog post in section 9 for
a worked example.

**Partial pivoting** (`determinant()`, `inverse()`). Gaussian elimination divides by the diagonal
('pivot') element. Dividing by a tiny pivot multiplies every rounding error already present by a
huge factor. Partial pivoting swaps rows so the division is always by the largest available
magnitude — each row swap flips the determinant's sign, which the code tracks. A pivot of exactly
zero after swapping means the matrix is singular, and `determinant()` returns 0.

Exact zeros are rare in floating point, though: a matrix that is singular on paper usually leaves a
pivot of rounding-noise size (try `[[0.1, 0.2, 0.3], [0.4, 0.5, 0.6], [0.7, 0.8, 0.9]]`), and
dividing by it produces an 'inverse' with entries around 10^15. So `inverse()` treats a pivot as
zero when it is within rounding error of zero *relative to the matrix's size*:
`|pivot| <= n * epsilon * ||A||_inf`, and returns `std::nullopt`, forcing the caller to handle the
case ('correct by construction' — there is no garbage result to use by accident). Because the test
is relative to the whole matrix, a badly scaled matrix such as `diag(1, 1e-300)` is also rejected;
keep rows and columns at comparable magnitudes (consistent units) before inverting.

A related subtlety: NaN compares false with *everything*. Validation code is therefore written as
`if (!(error <= tolerance)) reject;` rather than `if (error > tolerance) reject;` — the two differ
precisely when `error` is NaN, and only the first rejects it (`Rotation::fromMatrix()` in
[`frames.h`](../include/grape/linalg/frames.h)).

## 4. Rotations

### Two conventions, one headache

A rotation matrix can answer two different questions:

- **Active**: rotate this *object* within a fixed frame. Where did the point move to?
- **Passive**: rotate the *frame* and keep the object still. What are the same point's coordinates
  in the new frame?

The two matrices are transposes of each other, and mixing them up is the most common rotation bug
in robotics. This module is consistently **passive**, because sensor fusion and coordinate
transformation are overwhelmingly about re-expressing quantities in other frames:

```text
v_B = rx(angle) * v_A       where frame B is frame A rotated by angle about its x-axis
```

`rx`, `ry`, `rz` ([`frames.h`](../include/grape/linalg/frames.h)) are the three elementary
('direction cosine') matrices. Check the signs against your intuition: with a +90° rotation about
z, frame B's x-axis lies along frame A's y-axis, so A's x-axis seen from B points along -y: indeed
`rz(π/2) * (1,0,0) = (0,-1,0)`.

### Quaternions

A rotation matrix stores 9 numbers for 3 degrees of freedom, and numerical drift gradually breaks
its orthonormality. A **unit quaternion** stores 4 numbers with a single constraint (unit norm),
composes with fewer operations, and is trivial to re-normalise — which is why attitude estimators
integrate in quaternion form. [`quaternion.h`](../include/grape/linalg/quaternion.h) implements the
algebra of Trawny & Roumeliotis's reference report (equation numbers cited inline):
multiplication, conjugate, inverse, norm, and the conversion `rotationMatrix()` to the equivalent
passive matrix — the bridge between the quaternion and matrix worlds, with the same convention as
`rx/ry/rz` (verified against each other in [`frames_tests.cpp`](../tests/frames_tests.cpp)).

### Validated construction

Only some 3x3 matrices are rotations: the orthonormal ones with determinant +1 (determinant -1
means a *reflection* — a mirror image, not achievable by rotating anything).
`Rotation::fromMatrix()` checks both before accepting, and returns `std::nullopt` otherwise.
Composition and inversion preserve these properties by construction, so they skip revalidation —
though very long chains still accumulate drift.

## 5. Coordinate frames as types

### The notation

Furgale's insight is that every geometric quantity carries hidden bookkeeping, and writing it out
makes errors visible. The same bookkeeping, moved into C++ template parameters, makes errors
*impossible to compile*:

| Math          | C++                     | Meaning                                                    |
| ------------- | ----------------------- | ---------------------------------------------------------- |
| velocity v expressed in E | `Vector3<E>`   | Free vector: direction and magnitude, no anchor point      |
| r_AB expressed in E | `Displacement<E, A, B>` | Directed displacement from point A to point B, expressed in E |
| r_EP expressed in E | `Position<E, P>`  | Position of point P relative to E's origin, expressed in E |
| C_AB          | `Rotation<A, B>`        | Re-expresses B coordinates in A: v_A = C_AB * v_B          |
| T_AB          | `Transform<A, B>`       | Maps B-relative positions to A-relative positions          |

Frame tags derive from `Frame`, landmark tags from `PointTag`; a frame is also a point (its
origin). The tags occupy no storage — a `Vector3<World>` is exactly three doubles in memory, and
every check happens at compile time. Zero cost, by construction.

### The algebra

The permitted operations are the ones that make geometric sense, and only those:

- Free vectors with the *same basis* add, subtract, scale, dot and cross.
- Displacements chain head-to-tail: `r_AB + r_BC = r_AC`. Negation reverses the endpoints
  (`-r_AB = r_BA`). Nothing else: requiring path order is what catches mistakes.
- `Position - Position = Displacement` between the two points; `Position + Displacement` moves the
  endpoint. Positions cannot be added — 'Paris plus London' means nothing.
- Rotations compose along a chain: `C_AB * C_BC = C_AC`. The inner frames must match.
- A rotation re-expresses vectors and displacements (changing their basis, not their endpoints),
  but *not* positions: a position is anchored to an origin, and a rotation alone cannot move
  origins. Only a `Transform` can.

[`frames_tests.cpp`](../tests/frames_tests.cpp) turns the *forbidden* operations into
`static_assert`s too: concepts like `Addable` probe whether an expression compiles, so the type
system's refusals are themselves regression-tested.

## 6. Rigid transforms

A rigid body pose combines a rotation and a translation. `Transform<A, B>` stores `C_AB` and the
displacement `r_AB` of B's origin from A's origin, expressed in A, and maps positions as:

```text
r_AP = C_AB * r_BP + r_AB        (rotate into A's basis, then offset by B's origin)
```

Composition chains like rotations, with the translation picking up a rotated offset:

```text
T_AC = T_AB * T_BC:   C_AC = C_AB * C_BC,   r_AC = r_AB + C_AB * r_BC
```

The inverse swaps the frames: rotation `transpose(C_AB)` and translation
`-transpose(C_AB) * r_AB`. No matrix inversion is needed — the transpose *is* the inverse for an
orthonormal matrix, one of many places where preserving structure beats generic arithmetic.

Free vectors and displacements pass through a transform *without* translation: a velocity has no
position, so moving the origin does not change it. This distinction — automatic here, manual
everywhere else — is the classic 'points transform, vectors rotate' rule of computer graphics.

## 7. Concepts to code

| Concept                                   | Code                                                                 |
|-------------------------------------------|----------------------------------------------------------------------|
| Compile-time matrix dimensions            | `Matrix<NumRows, NumColumns, Scalar>` in [`matrix.h`](../include/grape/linalg/matrix.h) |
| Per-axis bounds checking                  | `flatIndex()` in [`matrix.h`](../include/grape/linalg/matrix.h)      |
| Cache-friendly multiplication             | `operator*` (i-k-j order) in [`matrix.h`](../include/grape/linalg/matrix.h) |
| Single-rounding accumulation              | `detail::multiplyAdd()` in [`matrix.h`](../include/grape/linalg/matrix.h) |
| Cancellation-safe cross product           | `differenceOfProducts()`, `cross()` in [`matrix.h`](../include/grape/linalg/matrix.h) |
| Partial pivoting, singularity as a type   | `determinant()`, `inverse()` in [`matrix.h`](../include/grape/linalg/matrix.h) |
| Quaternion algebra (Trawny)               | [`quaternion.h`](../include/grape/linalg/quaternion.h)               |
| Passive elementary rotations              | `rx()`, `ry()`, `rz()` in [`frames.h`](../include/grape/linalg/frames.h) |
| NaN-proof validation                      | `Rotation::fromMatrix()` in [`frames.h`](../include/grape/linalg/frames.h) |
| Frame bookkeeping as types                | `Vector3`, `Displacement`, `Position` in [`frames.h`](../include/grape/linalg/frames.h) |
| Destination-source chaining               | `Rotation::operator*`, `Transform::operator*` in [`frames.h`](../include/grape/linalg/frames.h) |
| Structure-preserving inverse              | `Rotation::inverse()`, `Transform::inverse()` in [`frames.h`](../include/grape/linalg/frames.h) |
| Forbidden operations as tests             | `Addable`/`Multipliable` probes in [`frames_tests.cpp`](../tests/frames_tests.cpp) |
| Worked scenario                           | [`example.cpp`](../examples/example.cpp)                             |


## Further reading

- P. Furgale, "Representing Robot Pose: The Good, the Bad, and the Ugly", tutorial, 2014-2025.
  <https://rpg.ifi.uzh.ch/docs/teaching/2025/FurgaleTutorial.pdf>. The notation and discipline
  behind section 5.
- N. Trawny, S. I. Roumeliotis, "Indirect Kalman Filter for 3D Attitude Estimation", Univ. of
  Minnesota TR-2005-002, 2005. The quaternion algebra of
  [`quaternion.h`](../include/grape/linalg/quaternion.h), equation by equation.
- M. Pharr, "The Difference of Floats", 2019.
  <https://pharr.org/matt/blog/2019/11/03/difference-of-floats>. Kahan's trick of section 3, with
  error analysis.
- wave_geometry frame semantics:
  <https://github.com/wavelab/wave_geometry/blob/master/docs/frame_semantics.md>. A more extensive
  implementation of typed frames.
- refx: <https://github.com/mosaico-labs/refx>. Typed geodetic frames (NED, ECEF, LLA) for
  navigation.
- "Robotics at Compile Time", CppCon 2023. <https://youtu.be/Y6AUsB3RUhA>. Compile-time robotics
  techniques in the same spirit.
- CSC4700 "Linear Algebra in C++" (LSU). <https://youtu.be/XzUTLsWyErA>. Where the multiplication
  loop order of section 2 leads: blocking, tiling and SIMD.
