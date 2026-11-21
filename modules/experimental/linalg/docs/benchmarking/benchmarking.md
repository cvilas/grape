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

Measured on AMD Ryzen AI Max+ 395, `x86-64-v3`, `-O2 -g`, GCC 16.0.1, Clang 23.1.6 (libc++), Eigen 3.4.0.

| Benchmark | linalg GCC | linalg Clang | Eigen GCC | Eigen Clang | What |
|---|---:|---:|---:|---:|:----|
| `multiply/3x3/f32` | 2.29 ns | 1.30 ns | 1.63 ns | 8.21 ns | _NxN_ Square matrix product |
| `multiply/4x4/f32` | 2.01 ns | 0.91 ns | 2.01 ns | 2.00 ns | .. |
| `multiply/6x6/f32` | 12.16 ns | 11.19 ns | 30.11 ns | 7.31 ns | .. |
| `determinant/3x3/f32` | 1.22 ns | 1.22 ns | 0.90 ns | 0.90 ns | Closed-form determinant |
| `determinant/4x4/f32` | 4.20 ns | 4.17 ns | 2.78 ns | 2.00 ns | .. |
| `determinant/15x15/f32` | 456.16 ns | 443.73 ns | 458.74 ns | 587.13 ns | Gaussian elimination |
| `inverse/4x4/f32` | 13.91 ns | 13.04 ns | 5.16 ns | 5.36 ns | General inverse |
| `inverse/15x15/f32` | 983.37 ns | 506.29 ns | 1,223 ns | 1,240 ns | .. |
| `quaternion_multiply/f32` | 1.30 ns | 1.59 ns | 0.81 ns | 0.72 ns | Attitude composition, matrix → quaternion |
| `quaternion_from_matrix/f32` | 1.69 ns | 2.09 ns | 3.18 ns | 2.03 ns | .. |
| `gyro_integration_latency/f32` | 14.87 ns | 13.76 ns | 12.58 ns | 12.03 ns | `q = normalized(q * dq)` as a dependent chain (latency) |
| `transform_position/f32` | 1.49 ns | 1.58 ns | 1.38 ns | 0.79 ns | `Transform * Position`; must match the untyped version |
| `batch_transform_10k/f32` | 9,098 ns | 9,826 ns | 12,753 ns | 3,893 ns | One transform over 10,000 points (throughput) |
| `kalman_step_15x6/f32` | 1,078 ns | 753.56 ns | 1,552 ns | 1,627 ns | Predict + update, 15 states, 6-D measurement (`eigen_ldlt` solves instead of inverting) |
| `multiply/3x3/f64` | 1.94 ns | 2.70 ns | 2.24 ns | 1.92 ns | .. |
| `multiply/4x4/f64` | 2.04 ns | 2.23 ns | 2.04 ns | 2.03 ns | .. |
| `multiply/6x6/f64` | 11.07 ns | 12.01 ns | 12.61 ns | 11.36 ns | .. |
| `determinant/3x3/f64` | 1.23 ns | 1.22 ns | 0.90 ns | 0.90 ns | .. |
| `determinant/4x4/f64` | 4.21 ns | 4.20 ns | 2.79 ns | 2.02 ns | .. |
| `determinant/15x15/f64` | 465.07 ns | 421.15 ns | 465.21 ns | 569.83 ns | .. |
| `inverse/4x4/f64` | 14.12 ns | 13.69 ns | 7.06 ns | 7.36 ns | .. |
| `inverse/15x15/f64` | 788.04 ns | 626.39 ns | 1,169 ns | 1,276 ns | .. |
| `quaternion_multiply/f64` | 1.31 ns | 1.27 ns | 0.86 ns | 1.00 ns | .. |
| `quaternion_from_matrix/f64` | 2.44 ns | 2.43 ns | 2.51 ns | 2.50 ns | .. |
| `gyro_integration_latency/f64` | 17.27 ns | 15.11 ns | 17.73 ns | 13.94 ns | .. |
| `transform_position/f64` | 1.49 ns | 1.59 ns | 2.08 ns | 0.81 ns | .. |
| `batch_transform_10k/f64` | 7,470 ns | 5,916 ns | 13,415 ns | 4,701 ns | .. |
| `kalman_step_15x6/f64` | 1,145 ns | 966.18 ns | 1,742 ns | 1,636 ns | .. |

## ARM64 baseline

Measured on an 8-core ARMv8.2-A system, `-O2 -g`, GCC 16.2.1, Clang 23.1.5 (libc++),

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
| `multiply/6x6/f64` | 63.83 ns | 20.81 ns | 12.40 ns | 9.80 ns | .. |
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

GCC at `-O2` vectorises only code shaped for it. It skips loops that need a scalar remainder or a
runtime alias check, and it keeps small arrays in registers only when their loops are fully
unrolled. Clang needs none of this. The workarounds are in
[`matrix.h`](../../include/grape/linalg/matrix.h):

- **Unroll hints** (GCC only) on element-wise and multiply loops. Removing them makes GCC 2–6×
  slower.
- **Two multiply kernels.** Small sizes unroll so the result stays in registers. Larger sizes
  accumulate each row in a local array (15×15 multiply 3.6× faster).
- **Gauss-Jordan reads a local copy of the pivot row**, and updates rows at full width (15×15
  inverse 1.7× faster on GCC).
- **Bounds-check failure is thrown out of line.** An inline `throw` stops GCC inlining element
  access in large functions (GCC batch transform 2.5× faster).
- **Closed-form inverse helpers are force-inlined.** Otherwise the adjugate is returned through
  memory (4×4 inverse up to 1.3× faster).

Ruled out: `#pragma omp simd` (not allowed in `constexpr` functions), and
`-fvect-cost-model=dynamic` (a build-wide flag that closes only part of the gap).

Known gaps: `-O3` makes GCC's 4×4 kernels 3–5× slower, and the 4×4 inverse is 2–3× slower than
Eigen's hand-written SIMD kernel.
