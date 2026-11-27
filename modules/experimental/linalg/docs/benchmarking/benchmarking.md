# Benchmarking: linalg

Benchmarks are in [`examples/bench.cpp`](../../examples/bench.cpp), named
`<operation>/<size>/<scalar>/<library>`. Each one is paired with an Eigen equivalent when Eigen is
found. Inputs are random, generated at runtime so that nothing is computed at compile time.

```bash
cmake --build build/native --target grape_linalg_bench
taskset -c 2 build/native/bin/grape_linalg_bench --benchmark_filter='kalman' \
  --benchmark_repetitions=5 --benchmark_report_aggregates_only=true --benchmark_min_time=0.2s
```

Code placement alone can move small kernels such as `inverse/4x4` by up to 30% between builds.
Before acting on a difference like that, confirm it with `-falign-functions=64 -falign-loops=64`.

## X86_64 baseline

Measured on AMD Ryzen AI Max+ 395, `x86-64-v3`, `-O3` (`Release`), GCC 16.0.1, Clang 23.1.6 (libc++), Eigen 3.4.0.

| Benchmark | linalg GCC | linalg Clang | Eigen GCC | Eigen Clang | What |
|---|---:|---:|---:|---:|:----|
| `multiply/3x3/f32` | 2.95 ns | 1.32 ns | 1.78 ns | 8.32 ns | _NxN_ Square matrix product |
| `multiply/4x4/f32` | 1.12 ns | 0.90 ns | 2.00 ns | 2.00 ns | .. |
| `multiply/6x6/f32` | 18.44 ns | 16.08 ns | 14.44 ns | 10.42 ns | .. |
| `determinant/3x3/f32` | 1.33 ns | 1.21 ns | 0.91 ns | 0.90 ns | Closed-form determinant |
| `determinant/4x4/f32` | 4.31 ns | 4.16 ns | 2.77 ns | 2.00 ns | .. |
| `determinant/15x15/f32` | 394.37 ns | 403.01 ns | 458.71 ns | 597.51 ns | Gaussian elimination |
| `inverse/4x4/f32` | 14.94 ns | 12.82 ns | 5.06 ns | 5.35 ns | General inverse |
| `inverse/15x15/f32` | 724.59 ns | 588.74 ns | 1,160 ns | 1,172 ns | .. |
| `quaternion_multiply/f32` | 1.42 ns | 1.59 ns | 0.85 ns | 0.72 ns | Attitude composition, matrix → quaternion |
| `quaternion_from_matrix/f32` | 1.92 ns | 2.15 ns | 1.80 ns | 1.54 ns | .. |
| `gyro_integration_latency/f32` | 16.28 ns | 13.91 ns | 12.75 ns | 12.16 ns | `q = normalized(q * dq)` as a dependent chain (latency) |
| `transform_position/f32` | 1.18 ns | 1.59 ns | 0.76 ns | 0.79 ns | `Transform * Position`; must match the untyped version |
| `batch_transform_10k/f32` | 4,421 ns | 9,026 ns | 4,154 ns | 3,857 ns | One transform over 10,000 points (throughput) |
| `kalman_step_15x6/f32` | 1,028 ns | 745.22 ns | 1,612 ns | 1,663 ns | Predict + update, 15 states, 6-D measurement (`eigen_ldlt` solves instead of inverting) |
| `multiply/3x3/f64` | 2.79 ns | 2.70 ns | 1.86 ns | 1.94 ns | .. |
| `multiply/4x4/f64` | 2.14 ns | 2.22 ns | 2.22 ns | 2.03 ns | .. |
| `multiply/6x6/f64` | 12.19 ns | 11.37 ns | 11.99 ns | 14.14 ns | .. |
| `determinant/3x3/f64` | 1.26 ns | 1.22 ns | 0.90 ns | 0.90 ns | .. |
| `determinant/4x4/f64` | 4.20 ns | 4.18 ns | 2.87 ns | 2.01 ns | .. |
| `determinant/15x15/f64` | 429.48 ns | 415.50 ns | 446.11 ns | 597.54 ns | .. |
| `inverse/4x4/f64` | 15.80 ns | 13.06 ns | 6.56 ns | 7.34 ns | .. |
| `inverse/15x15/f64` | 824.04 ns | 757.76 ns | 1,369 ns | 1,281 ns | .. |
| `quaternion_multiply/f64` | 1.31 ns | 1.26 ns | 0.86 ns | 1.02 ns | .. |
| `quaternion_from_matrix/f64` | 2.43 ns | 2.43 ns | 2.42 ns | 2.46 ns | .. |
| `gyro_integration_latency/f64` | 17.46 ns | 15.67 ns | 17.93 ns | 15.58 ns | .. |
| `transform_position/f64` | 1.50 ns | 1.60 ns | 0.75 ns | 0.82 ns | .. |
| `batch_transform_10k/f64` | 6,950 ns | 5,335 ns | 4,631 ns | 4,653 ns | .. |
| `kalman_step_15x6/f64` | 987.42 ns | 929.33 ns | 1,654 ns | 1,640 ns | .. |

## ARM64 baseline

Measured on an 8-core ARMv8.2-A system, `-O2 -g`, GCC 16.2.1, Clang 23.1.5 (libc++). Not yet
re-measured at `-O3` or since the kernels were simplified.

| Benchmark | linalg GCC | linalg Clang | Eigen GCC | Eigen Clang | What |
|---|---:|---:|---:|---:|:----|
| `multiply/3x3/f32` | 2.84 ns | 3.09 ns | 2.55 ns | 2.14 ns | _NxN_ Square matrix product |
| `multiply/4x4/f32` | 2.02 ns | 1.90 ns | 1.85 ns | 1.61 ns | .. |
| `multiply/6x6/f32` | 9.48 ns | 11.21 ns | 10.96 ns | 9.97 ns | .. |
| `determinant/3x3/f32` | 1.26 ns | 1.26 ns | 0.91 ns | 0.95 ns | Closed-form determinant |
| `determinant/4x4/f32` | 4.50 ns | 4.53 ns | 2.42 ns | 2.41 ns | .. |
| `determinant/15x15/f32` | 614.42 ns | 455.04 ns | 573.39 ns | 752.51 ns | Gaussian elimination |
| `inverse/4x4/f32` | 15.26 ns | 18.50 ns | 7.38 ns | 9.71 ns | General inverse |
| `inverse/15x15/f32` | 1,918.86 ns | 732.55 ns | 1,887.97 ns | 1,927.63 ns | .. |
| `quaternion_multiply/f32` | 1.50 ns | 1.49 ns | 0.99 ns | 1.65 ns | Attitude composition, matrix → quaternion |
| `quaternion_from_matrix/f32` | 2.81 ns | 3.03 ns | 1.64 ns | 1.99 ns | .. |
| `gyro_integration_latency/f32` | 22.11 ns | 23.05 ns | 18.12 ns | 17.76 ns | Dependent quaternion integration chain |
| `transform_position/f32` | 1.36 ns | 1.24 ns | 2.57 ns | 0.90 ns | `Transform * Position` |
| `batch_transform_10k/f32` | 10,229.35 ns | 3,812.50 ns | 33,291.09 ns | 6,875.66 ns | One transform over 10,000 points |
| `kalman_step_15x6/f32` | 1,270.73 ns | 960.66 ns | 1,934.55 ns | 1,751.80 ns | Predict + update, 15 states, 6-D measurement |
| `multiply/3x3/f64` | 2.84 ns | 2.48 ns | 2.64 ns | 2.10 ns | .. |
| `multiply/4x4/f64` | 3.17 ns | 3.05 ns | 14.82 ns | 2.72 ns | .. |
| `multiply/6x6/f64` | 24.2 ns | 17.9 ns | 12.40 ns | 9.80 ns | .. |
| `determinant/3x3/f64` | 1.26 ns | 1.26 ns | 0.91 ns | 0.95 ns | .. |
| `determinant/4x4/f64` | 4.50 ns | 4.53 ns | 2.42 ns | 2.41 ns | .. |
| `determinant/15x15/f64` | 625.64 ns | 523.82 ns | 621.87 ns | 735.95 ns | .. |
| `inverse/4x4/f64` | 25.71 ns | 18.51 ns | 8.36 ns | 8.93 ns | .. |
| `inverse/15x15/f64` | 1,950.33 ns | 1,142.94 ns | 1,866.09 ns | 1,796.36 ns | .. |
| `quaternion_multiply/f64` | 1.51 ns | 1.49 ns | 1.24 ns | 1.49 ns | .. |
| `quaternion_from_matrix/f64` | 2.98 ns | 3.16 ns | 1.82 ns | 1.74 ns | .. |
| `gyro_integration_latency/f64` | 24.02 ns | 24.27 ns | 22.24 ns | 19.01 ns | .. |
| `transform_position/f64` | 1.36 ns | 1.24 ns | 2.11 ns | 1.10 ns | .. |
| `batch_transform_10k/f64` | 10,277.41 ns | 7,629.43 ns | 28,286.77 ns | 7,693.25 ns | .. |
| `kalman_step_15x6/f64` | 1,696.99 ns | 1,407.38 ns | 2,577.85 ns | 2,156.91 ns | .. |

## Implementation notes

Kernels are plain loops that GCC and Clang vectorise at `-O3`. The remaining workarounds are in
[`matrix.h`](../../include/grape/linalg/matrix.h):

- **Bounds-check failure is thrown out of line.** An inline `throw` stops GCC inlining element
  access (GCC `quaternion_from_matrix` 3–4× faster).
- **Closed-form adjugates are force-inlined.** `determinant()` reuses them and relies on the unused
  terms being optimised away (4×4 determinant 2× faster).
- **Tagged coordinates are copied elementwise.** GCC does not vectorise loops that copy a whole
  `Vector` into a member (GCC `batch_transform_10k/f32` 1.9× faster).
- **`Transform * Position` accumulates onto the translation** instead of adding it last (one
  rounding fewer; batch transform ~10% faster).
- **Multiply accumulates each row in a local vector** (GCC 6×6 `f64` 1.5× faster than
  accumulating into the result).

Known gaps: against the former `-O2` build with GCC-specific unroll hints, 6×6 multiply (both
compilers) and Clang's 15×15 inverse are 15–50% slower. The 4×4 inverse is 2–3× slower than Eigen's
hand-written SIMD kernel. `transform_position` is 1.5–2× slower than Eigen, whose padded 4×4 isometry
maps a point with three 4-lane FMAs. Clang does not vectorise `batch_transform_10k` across points
(2.3× slower than Eigen).
