# Linear algebra for rigid body motion: a primer

This primer explains the design of the `linalg` module from first principles. It assumes secondary
school linear algebra (matrix multiplication, determinants) and nothing about robotics conventions.
Each section links to the code that implements it, so you can read the two together.

Suggested reading order:

1. Sections 1 to 3 (matrices and the numerical techniques), with
   [`matrix.h`](../include/grape/linalg/matrix.h)
2. Section 4 (rotations), with [`quaternion.h`](../include/grape/linalg/quaternion.h) and the top
   of [`frames.h`](../include/grape/linalg/frames.h)
3. Sections 5 and 6 (coordinate frames as types), with the rest of
   [`frames.h`](../include/grape/linalg/frames.h) and
   [`frames_tests.cpp`](../tests/frames_tests.cpp)
4. Section 8 (experiments to try)

---

## 1. The big idea

Most geometry bugs in robotics are not arithmetic errors. They are *bookkeeping* errors: a vector
expressed in the camera frame added to one expressed in the world frame, a rotation applied in the
wrong direction, a position treated as if it were a direction. The arithmetic is perfectly happy to
multiply the numbers; the result is silently wrong, and the robot drives into a wall. The loss of
the Mars Climate Orbiter (1999) was exactly this class of error, in unit rather than frame
bookkeeping.

The numbers cannot catch these errors, but the *types* can. This module is built in two layers:

- **A numeric kernel** ([`matrix.h`](../include/grape/linalg/matrix.h),
  [`quaternion.h`](../include/grape/linalg/quaternion.h)): plain matrices, vectors and quaternions
  with carefully implemented arithmetic. Dimensions are template parameters, so a 3x4 times 2x2
  multiplication is a compile error.
- **A frame-aware layer** ([`frames.h`](../include/grape/linalg/frames.h)): wrappers that add
  *which frame, which points* to each quantity as zero-cost type tags. Mixing incompatible frames
  becomes a compile error too.

Everything is `constexpr`: the compiler can evaluate entire computations at compile time, which is
used in the tests ([`matrix_tests.cpp`](../tests/matrix_tests.cpp)) to verify results with
`static_assert` — a failed check stops the build, not the robot.

The conventions follow P. Furgale, *Representing Robot Pose: The Good, the Bad, and the Ugly*
(section 5). Prior art for the idea includes
[wave_geometry](https://github.com/wavelab/wave_geometry/blob/master/docs/frame_semantics.md) and
[refx](https://github.com/mosaico-labs/refx).

## 2. Matrices from first principles

A `Matrix<NumRows, NumColumns, Scalar>` ([`matrix.h`](../include/grape/linalg/matrix.h)) stores its
elements in one flat `std::array`, row by row ('row-major'). Element (r, c) lives at flat index
`r * NumColumns + c`. There is no heap allocation, no pointer chasing, and the size is part of the
type:

- The compiler knows every loop bound, so it can unroll and vectorise freely. For the 3x3 and 4x4
  matrices of rigid body motion, the generated code is a handful of instructions.
- Shape errors are compile errors: `operator*` only accepts a right operand whose row count equals
  the left operand's column count.
- A `Vector<N>` is just a `Matrix<N, 1>` — one definition, one set of operations (the README's
  'defined in terms of Matrix').

Element access `mat[row, col]` (a C++23 multidimensional subscript) is bounds-checked on *each
axis*: a column index past the end would otherwise wrap around to the next row — within the flat
array, and therefore invisible to a flat bounds check (`flatIndex()` in
[`matrix.h`](../include/grape/linalg/matrix.h)).

Matrix multiplication (`operator*`) iterates in `i-k-j` order rather than the textbook `i-j-k`.
Both compute the same sum; the difference is memory access. With `j` innermost, both `result[i, j]`
and `rhs[k, j]` walk along a row — consecutive memory — while the textbook order makes `rhs[k, j]`
jump a whole row ahead on every step. For the small matrices here either is fast; the habit matters
when the same code grows blocks, tiles, and SIMD (see the 'Stellar' lecture in section 9).

## 3. Numerical care

Floating point numbers have finite precision, and the error of a long computation is dominated by a
few well-known traps. Three techniques, all in [`matrix.h`](../include/grape/linalg/matrix.h),
avoid them:

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
zero after swapping means the matrix is singular: `determinant()` returns 0 and `inverse()` returns
`std::nullopt`, forcing the caller to handle the case ('correct by construction' — there is no
garbage result to use by accident).

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

### What the compiler cannot do

The compiler proves compatibility of *declared* semantics, not physical truth. It cannot know that
raw coordinates passed to the `Vector3<World>` constructor actually came from the camera. Raw
numbers enter through explicit constructors and leave through `coordinates()` — a deliberate,
visible escape hatch at the boundary of the typed world.

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

## 8. Experiments

Change one thing, rebuild, and observe. Each isolates an idea from above.

1. **Break the frame chain.** In [`example.cpp`](../examples/example.cpp), change `pose_wb *
   pose_bc` to `pose_bc * pose_wb` and read the compile error. This is the bug the module exists to
   catch; note that it is caught before the program ever runs.
2. **See catastrophic cancellation.** Compute `a*b - c*d` naively for `a = 1 + 2e-8`, `b = 1 -
   2e-8`, `c = d = 1` in `float`, and compare with `differenceOfProducts()`. The naive answer loses
   most of its digits.
3. **Feel the pivoting.** In `inverse()`, replace pivot selection with `pivot_row = col` (no
   search) and run the 'pivots rows' test in [`matrix_tests.cpp`](../tests/matrix_tests.cpp): a
   perfectly invertible matrix is now rejected because of a zero on the diagonal.
4. **Prove it at compile time.** Add
   `static_assert(trace(identity<5, int>()) == 5);` anywhere. The computation — loops and all —
   runs inside the compiler.
5. **Catch a reflection.** Build a rotation matrix with one axis negated and pass it to
   `Rotation::fromMatrix()`. Orthonormal, but determinant -1: rejected.

## 9. Further reading

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
