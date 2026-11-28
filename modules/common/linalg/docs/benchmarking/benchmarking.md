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
| `batch_transform_10k/f32` | 4,443 ns | 9,019 ns | 4,183 ns | 3,894 ns | One transform over 10,000 points (throughput) |
| `batch_transform_10k/f32/map` | 4,441 ns | 3,020 ns | - | - | The same through `Transform::map()` |
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
| `batch_transform_10k/f64` | 6,958 ns | 5,353 ns | 8,089 ns | 4,793 ns | .. |
| `batch_transform_10k/f64/map` | 7,019 ns | 5,361 ns | - | - | .. |
| `kalman_step_15x6/f64` | 987.42 ns | 929.33 ns | 1,654 ns | 1,640 ns | .. |

## ARM64 baseline

Measured on an Apple M1 Pro, `ARMv8.2-A`, pinned to one performance core, `-O3` (`Release`), GCC
16.2.1, Clang 23.1.6 (libc++), Eigen 5.0.1-dev.

| Benchmark | linalg GCC | linalg Clang | Eigen GCC | Eigen Clang | What |
|---|---:|---:|---:|---:|:----|
| `multiply/3x3/f32` | 2.78 ns | 3.11 ns | 1.82 ns | 2.15 ns | _NxN_ Square matrix product |
| `multiply/4x4/f32` | 2.98 ns | 1.90 ns | 1.87 ns | 1.62 ns | .. |
| `multiply/6x6/f32` | 23.27 ns | 10.61 ns | 11.10 ns | 11.10 ns | .. |
| `determinant/3x3/f32` | 1.27 ns | 1.27 ns | 0.92 ns | 0.96 ns | Closed-form determinant |
| `determinant/4x4/f32` | 4.54 ns | 4.63 ns | 2.46 ns | 2.42 ns | .. |
| `determinant/15x15/f32` | 391.39 ns | 458.80 ns | 571.66 ns | 704.91 ns | Gaussian elimination |
| `inverse/4x4/f32` | 18.80 ns | 17.51 ns | 7.49 ns | 9.78 ns | General inverse |
| `inverse/15x15/f32` | 1,054 ns | 919.12 ns | 1,587 ns | 1,854 ns | .. |
| `quaternion_multiply/f32` | 1.52 ns | 1.51 ns | 1.01 ns | 1.66 ns | Attitude composition, matrix → quaternion |
| `quaternion_from_matrix/f32` | 2.86 ns | 2.79 ns | 2.96 ns | 3.11 ns | .. |
| `gyro_integration_latency/f32` | 22.40 ns | 22.83 ns | 18.36 ns | 17.84 ns | `q = normalized(q * dq)` as a dependent chain (latency) |
| `transform_position/f32` | 1.34 ns | 1.20 ns | 0.80 ns | 0.91 ns | `Transform * Position`; must match the untyped version |
| `batch_transform_10k/f32` | 3,350 ns | 3,324 ns | 6,509 ns | 6,969 ns | One transform over 10,000 points (throughput) |
| `batch_transform_10k/f32/map` | 3,349 ns | 3,327 ns | - | - | The same through `Transform::map()` |
| `kalman_step_15x6/f32` | 1,161 ns | 934.67 ns | 1,904 ns | 1,953 ns | Predict + update, 15 states, 6-D measurement (`eigen_ldlt` solves instead of inverting) |
| `multiply/3x3/f64` | 2.80 ns | 2.50 ns | 1.86 ns | 2.16 ns | .. |
| `multiply/4x4/f64` | 5.75 ns | 3.14 ns | 2.94 ns | 13.28 ns | .. |
| `multiply/6x6/f64` | 24.81 ns | 18.26 ns | 12.79 ns | 9.89 ns | .. |
| `determinant/3x3/f64` | 1.28 ns | 1.29 ns | 0.92 ns | 0.97 ns | .. |
| `determinant/4x4/f64` | 4.55 ns | 4.67 ns | 2.46 ns | 2.44 ns | .. |
| `determinant/15x15/f64` | 422.40 ns | 545.84 ns | 627.20 ns | 703.41 ns | .. |
| `inverse/4x4/f64` | 21.87 ns | 17.75 ns | 8.06 ns | 9.06 ns | .. |
| `inverse/15x15/f64` | 1,524 ns | 1,422 ns | 1,746 ns | 1,795 ns | .. |
| `quaternion_multiply/f64` | 1.54 ns | 1.51 ns | 1.25 ns | 1.50 ns | .. |
| `quaternion_from_matrix/f64` | 2.91 ns | 2.99 ns | 2.51 ns | 2.46 ns | .. |
| `gyro_integration_latency/f64` | 24.41 ns | 24.76 ns | 20.53 ns | 19.07 ns | .. |
| `transform_position/f64` | 1.34 ns | 1.21 ns | 1.17 ns | 1.11 ns | .. |
| `batch_transform_10k/f64` | 6,645 ns | 6,636 ns | 7,086 ns | 7,729 ns | .. |
| `batch_transform_10k/f64/map` | 6,644 ns | 6,634 ns | - | - | .. |
| `kalman_step_15x6/f64` | 1,535 ns | 1,393 ns | 2,474 ns | 2,330 ns | .. |

## Implementation notes

Kernels are plain loops that GCC and Clang vectorise at `-O3`. The remaining workarounds are in
[`matrix.h`](../../include/grape/linalg/matrix.h) and
[`frames.h`](../../include/grape/linalg/frames.h):

- **Bounds-check failure is thrown out of line.** An inline `throw` stops GCC inlining element
  access (GCC `quaternion_from_matrix` 3–4× faster).
- **Closed-form adjugates are force-inlined.** `determinant()` reuses them and relies on the unused
  terms being optimised away (4×4 determinant 2× faster).
- **Tagged coordinates are copied elementwise.** GCC does not vectorise loops that copy a whole
  `Vector` into a member (GCC `batch_transform_10k/f32` 1.9× faster).
- **`Transform * Position` accumulates onto the translation** instead of adding it last (one
  rounding fewer; batch transform ~10% faster).
- **`Transform::map()` stores each coordinate on its own.** Clang does not vectorise a loop that
  stores whole `Position`s (Clang `batch_transform_10k/f32` 3× faster than mapping point by point).
- **Multiply accumulates each row in a local vector** (GCC 6×6 `f64` 1.5× faster than
  accumulating into the result).

Known gaps: against the former `-O2` build with GCC-specific unroll hints, 6×6 multiply (both
compilers) and Clang's 15×15 inverse are 15–50% slower. The 4×4 inverse is 2–3× slower than Eigen's
hand-written SIMD kernel. `transform_position` is 1.5–2× slower than Eigen, whose padded 4×4
isometry maps a point with three 4-lane FMAs.
