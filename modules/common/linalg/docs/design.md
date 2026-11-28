# Design notes: linalg

Most geometry bugs are bookkeeping errors (a vector in the wrong frame, a rotation applied the wrong
way), not arithmetic errors. linalg catches them at compile time in two layers:

- **Numeric kernel** ([`matrix.h`](../include/grape/linalg/matrix.h),
  [`quaternion.h`](../include/grape/linalg/quaternion.h)): fixed-size matrices, vectors and
  quaternions. Dimension mismatches do not compile.
- **Frame layer** ([`frames.h`](../include/grape/linalg/frames.h)): geometric types tagged with
  frames and points. The tags take no storage, and frame mismatches do not compile.

## Conventions

Notation and rules follow [Furgale](./furgale.pdf).

| Math | C++ | Meaning |
|---|---|---|
| v expressed in E | `Vector3<E>` | Free vector (direction, velocity): no anchor point |
| r_AB expressed in E | `Displacement<E, A, B>` | From point A to point B |
| r_EP expressed in E | `Position<E, P>` | Point P relative to E's origin |
| C_AB | `Rotation<A, B>` | Re-expresses B coordinates in A: v_A = C_AB v_B |
| T_AB | `Transform<A, B>` | Maps B-relative positions to A-relative: r_AP = C_AB r_BP + r_AB |

- **Rotations are passive**: they re-express a quantity in another frame and do not move it. For
  example, `rz(π/2) * (1,0,0) = (0,-1,0)`.
- **Template order is destination, source**: `Rotation<To, From>`. This order chains:
  `C_AB * C_BC = C_AC`.
- **Only operations that make geometric sense compile.** Displacements chain head to tail
  (`r_AB + r_BC = r_AC`). `Position - Position` gives a `Displacement`. Positions cannot be added.
  Rotations act on vectors and displacements but not on positions, because only a `Transform` can
  move an origin.
- **Quaternions** follow [Trawny & Roumeliotis](./trawny.pdf): layout `[x, y, z, w]`, and `p * q`
  applies `q` first, so `C(p * q) = C(p) C(q)`.

## Numerics

- **FMA**: every accumulation uses fused multiply-add, which rounds once instead of twice. This
  requires hardware FMA (`x86-64-v3`, `armv8.2-a`).
- **`a*b - c*d`** uses Kahan's `differenceOfProducts` ([kahan](./kahan.pdf)), which keeps a few ulp
  of accuracy under cancellation. Cross products, determinants and adjugates use it.
- **Elimination uses partial pivoting.**
- **Singularity is relative to the matrix's scale**: `inverse()` returns `std::nullopt` when a
  pivot is at most n·ε·‖A‖∞. Badly scaled matrices such as `diag(1, 1e-300)` are rejected, so
  rescale them to comparable magnitudes first.
- **`inverse()` never returns inf/NaN**: an unrepresentable inverse is `std::nullopt`.
- **Norms are not range-safe**: `norm()` and quaternion `normalized()`/`inverse()` square
  components directly (as Eigen's `norm()` does), so they fail beyond ~1e±154 (`double`) or
  ~1e±19 (`float`).
- **Validation also rejects NaN**: checks are written `if (!(err <= tol)) reject`.
- **`Rotation::fromMatrix()` validates its input**: the matrix must be orthonormal with determinant
  +1. Composition and inversion do not revalidate, so long chains accumulate drift.

Performance: see [benchmarking](./benchmarking/benchmarking.md).

## Positioning: linalg vs Eigen for robotics

Eigen is far ahead as a general robotics maths library. linalg's distinguishing feature is the
compile-time frame layer.

| Area | linalg | Eigen | Notes |
|---|---|---|---|
| **Frame safety** | ★★★★★ | ★☆☆☆☆ | Mixing up `T_AB` and `T_BA` is a compile error. Eigen, Sophus and manif have no frames. |
| **Clarity** | ★★★★★ | ★★☆☆☆ | ~1,000 readable lines tied to Trawny and Furgale conventions. |
| **Compile-time evaluation** | ★★★★☆ | ★☆☆☆☆ | Real `static_assert` proofs (sqrt and trig only on GCC). |
| **Numerical care (small sizes)** | ★★★★☆ | ★★★★☆ | FMA accumulation and accurate cross products vs Eigen's maturity. |
| **Performance** | ★★★★☆ | ★★★★★ | On par at small multiplies, ahead at the 15-state Kalman step; behind on 4×4 inverse and at large or dynamic sizes. See [benchmarking.md](./docs/benchmarking/benchmarking.md). |
| **Breadth** | ★☆☆☆☆ | ★★★★★ | No solvers, Cholesky/LDLT, QR, SVD, eigen-decomposition, slerp, dynamic or sparse matrices. |
| **Estimation** | ★☆☆☆☆ | ★★★★★ | No SO(3)/SE(3) exp/log, Jacobians, ⊕/⊖. `Scalar` excludes autodiff types by design. |
| **Ecosystem** | ☆☆☆☆☆ | ★★★★★ | ROS, Ceres, GTSAM, Pinocchio, Drake, OpenCV and PCL all use Eigen types. |
| **Realtime fit** | ★★★☆☆ | ★★★★☆ | Element access **throws** on a bad index; breaks under `-fno-exceptions`. |

- **Good fit:** small embedded attitude and pose code (ahrs on a Pi), small Kalman filters, frame conversion glue. 
- **Poor fit:** SLAM/VIO, trajectory optimisation, manipulator dynamics, ROS types, filters that need a Cholesky solve.

## References

- P. Furgale, [Representing Robot Pose: The Good, the Bad, and the Ugly](https://rpg.ifi.uzh.ch/docs/teaching/2025/FurgaleTutorial.pdf)
- N. Trawny, S. I. Roumeliotis, *Indirect Kalman Filter for 3D Attitude Estimation*, TR-2005-002, 2005
- M. Pharr, [The Difference of Floats](https://pharr.org/matt/blog/2019/11/03/difference-of-floats)
- [wave_geometry frame semantics](https://github.com/wavelab/wave_geometry/blob/master/docs/frame_semantics.md), [refx](https://github.com/mosaico-labs/refx): other typed-frame libraries
- [Robotics at Compile Time](https://youtu.be/Y6AUsB3RUhA), CppCon 2023
