# TODO - linalg

> Remove before merging `feat/linalg` branch

- Full review of linalg
- Review [doc](./docs/frames_and_numerics.md)
- Should Vector have `.x`, `.y`, `.z` accessors like quaternion

## Open questions from the review

- [ ] **`inverse()` threshold.** `n·ε·‖A‖∞` is a heuristic. For Kalman covariances with mixed units,
  decide whether callers should rescale first or a condition estimate is wanted.
- [ ] **`TaggedCoordinates` inheritance.** The no-cross-tag-conversion guarantee rests on
  inherited-constructor rules. `static_assert`s pin it, but it's subtle.


## Performance

- [ ] **`-O3` (Release) slows GCC's small kernels**: 4×4 inverse 14 → 37–68 ns, 4×4 f32 multiply
  2.0 → 7.5 ns. Not explained by the vectoriser cost model or either vectoriser.
- [ ] **`batch_transform_10k/f32` on GCC** is 21 µs vs 10 µs on Clang (the caller's point loop isn't
  vectorised), and 3× slower than the identical-looking f64 loop. Unexplained; needs `perf`.
- [ ] **4×4 inverse is ~2× behind Eigen** (scalar closed form vs Eigen's SIMD kernel).
- [ ] **Quaternion multiply is ~1.6× behind Eigen** (serial `multiplyAdd` chain). Low priority.

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
| **Breadth** | ★☆☆☆☆ | ★★★★★ | No solve, Cholesky/LDLT, QR, SVD, eigen-decomposition, slerp, dynamic or sparse matrices. |
| **Estimation** | ★☆☆☆☆ | ★★★★★ | No SO(3)/SE(3) exp/log, Jacobians, ⊕/⊖. `Scalar` excludes autodiff types by design. |
| **Ecosystem** | ☆☆☆☆☆ | ★★★★★ | ROS, Ceres, GTSAM, Pinocchio, Drake, OpenCV and PCL all use Eigen types. |
| **Realtime fit** | ★★★☆☆ | ★★★★☆ | Element access **throws** on a bad index; breaks under `-fno-exceptions`. |

**Good fit:** small embedded attitude and pose code (ahrs on a Pi), small Kalman filters, frame
conversion glue. **Poor fit:** SLAM/VIO, trajectory optimisation, manipulator dynamics, ROS types,
filters that need a Cholesky solve.

**Deciding question:** will grape need to work with ROS, Ceres or GTSAM? If so, choose option 1 early.

1. **Frames on top of Eigen.** Make the frame types generic over storage and back them with Eigen
   (or Sophus). Keeps frame safety; gains Eigen's speed, breadth and interop.
2. **Stay dependency-free and close the gaps that matter**, in priority order:
   - [ ] LDLT or Cholesky solve
   - [ ] SO(3)/SE(3) exp/log with Jacobians
   - [ ] slerp
   - [ ] loosen the `Scalar` constraint so autodiff types work
   - [ ] a no-throw option for element access
   - [ ] hand-vectorised 3×3 and 4×4 kernels

## Eigen interop and backends

- [ ] **Interop adapters** in `grape/linalg/eigen.h`, included only by code that wants them:
  `asEigen()` (zero-copy `Eigen::Map` over `elements()`; column vectors must map as ColMajor),
  `fromEigen()`, and quaternion conversions handling JPL ↔ Hamilton explicitly.
- [ ] **Optional Eigen backend**, only if a gap matters on the Pi 5:
  - One project-wide CMake option (`GRAPE_LINALG_USE_EIGEN`) as an INTERFACE definition; mixing
    modes across translation units is an ODR violation. Don't auto-enable when Eigen is installed.
  - Keep `Matrix` a row-major `std::array`; Eigen sees it only through `Eigen::Map`.
  - Call Eigen only under `if !consteval`, so compile-time evaluation keeps working.
  - Dispatch per kernel and size from benchmark data: Eigen wins the 4×4 inverse, linalg wins the
    15×15 inverse and Kalman step on both compilers.
  - Test both modes, with a cross-check against the built-in kernels.
- [ ] **`std::linalg` (C++26, P1673)** as a kernel backend once toolchains ship it. BLAS-level only
  (products, triangular solves), no inverses or decompositions.

Alternatives considered: glm (≤ 4×4, graphics-oriented), Fastor (fast but small community), Blaze (as
heavy as Eigen), sgorsten/linalg.h (≤ 4×4, no SIMD), xtensor/Armadillo (dynamic sizes). None is
simpler, as fast and as broad as Eigen.
