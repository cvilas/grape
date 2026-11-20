# Benchmarking: linalg

- See [`examples/bench.cpp`](../../examples/bench.cpp). 
- If Eigen is found, each benchmark is paired with an Eigen equivalent in the same executable.
- Run benchmarks as follows
  ```bash
  cmake --build build/native --target grape_linalg_bench
  build/native/bin/grape_linalg_bench --benchmark_filter='kalman'   # optional filter

  # stable numbers: pinned core, repeated, medians only
  taskset -c 2 build/native/bin/grape_linalg_bench \
    --benchmark_repetitions=5 --benchmark_report_aggregates_only=true --benchmark_min_time=0.2s
  ```

## Baseline

- Names are `<operation>/<size>/<scalar>/<library>`. Inputs are random but fixed, generated at runtime and
passed through `DoNotOptimize` so nothing is computed at compile time.
- Measured on AMD Ryzen AI Max+ 395, `x86-64-v4`, `-O2 -g`, GCC 16.0.1, Clang 23.1.6 (libc++), Eigen 3.4.0. Medians of
5 runs, one pinned core, 2026-11-18. `quaternion_from_matrix` is noisy (up to 52% variation); everything
else is within 7%.

| Benchmark | linalg GCC | linalg Clang | Eigen GCC | Eigen Clang | What |
|---|---:|---:|---:|---:|:----|
| `multiply/3x3/f32` | 2.33 ns | 1.33 ns | 1.66 ns | 8.34 ns | _NxN_ Square matrix product |
| `multiply/4x4/f32` | 2.03 ns | 0.94 ns | 2.04 ns | 2.05 ns | .. |
| `multiply/6x6/f32` | 11.84 ns | 11.37 ns | 30.65 ns | 7.46 ns | .. |
| `inverse/4x4/f32` | 14.48 ns | 11.54 ns | 5.23 ns | 5.50 ns | General inverse |
| `inverse/15x15/f32` | 992.54 ns | 516.67 ns | 1,300 ns | 1,170 ns | .. |
| `quaternion_multiply/f32` | 1.33 ns | 1.61 ns | 0.82 ns | 0.74 ns | Attitude composition, matrix → quaternion |
| `quaternion_from_matrix/f32` | 2.65 ns | 2.10 ns | 3.31 ns | 2.07 ns | .. |
| `gyro_integration_latency/f32` | 15.25 ns | 13.99 ns | 12.83 ns | 12.27 ns | `q = normalized(q * dq)` as a dependent chain (latency) |
| `transform_position/f32` | 2.25 ns | 1.63 ns | 1.41 ns | 0.81 ns  | `Transform * Position`; must match the untyped version |
| `batch_transform_10k/f32` | 21,370 ns | 10,040 ns | 13,120 ns | 3,950 ns | One transform over 10,000 points (throughput) |
| `kalman_step_15x6/f32` | 1,140 ns | 770.03 ns | 1,570 ns | 1,680 ns | Predict + update, 15 states, 6-D measurement (`eigen_ldlt` solves instead of inverting) |
| `multiply/3x3/f64` | 2.03 ns | 2.75 ns | 2.27 ns | 1.97 ns | .. |
| `multiply/4x4/f64` | 2.08 ns | 2.27 ns | 2.08 ns | 2.10 ns | .. |
| `multiply/6x6/f64` | 11.33 ns | 12.19 ns | 12.86 ns | 11.60 ns | .. |
| `inverse/4x4/f64` | 14.32 ns | 12.65 ns | 7.18 ns | 7.46 ns | .. |
| `inverse/15x15/f64` | 805.15 ns | 619.65 ns | 1,130 ns | 1,400 ns | .. |
| `quaternion_multiply/f64` | 1.33 ns | 1.28 ns | 0.87 ns | 1.03 ns | .. |
| `quaternion_from_matrix/f64` | 3.07 ns | 2.47 ns | 3.43 ns | 2.48 ns | .. |
| `gyro_integration_latency/f64` | 17.55 ns | 15.38 ns | 18.09 ns | 14.38 ns | .. |
| `transform_position/f64` | 2.07 ns | 1.61 ns | 2.11 ns | 0.82 ns | .. |
| `batch_transform_10k/f64` | 7,580 ns | 6,060 ns | 13,780 ns | 4,880 ns | .. |
| `kalman_step_15x6/f64` | 1,110 ns | 964.75 ns | 1,770 ns | 1,650 ns | .. |

## Implementation tricks

Clang vectorises plain loops better than GCC. At `-O2` GCC needs the code shaped for it: its default vectoriser cost
model refuses loops that need a scalar remainder (any 15-wide loop) or a runtime alias check, and it only
splits small local arrays into registers once their loops are fully unrolled. These tricks are in
[`matrix.h`](../../include/grape/linalg/matrix.h); the unroll hints apply to GCC only.

- **Closed forms for small inverses and determinants** (orders 2–4)
- **Element-wise operators as indexed loops with an unroll hint**, not `std::ranges::transform`. Lets GCC
  keep small temporaries in registers (f64 batch transform 6× faster).
- **Two multiply kernels by size.** Up to 8 rows and inner dimension: unroll the two outer loops and let
  the contiguous inner loop vectorise. Larger: accumulate each result row in a local array with the
  column loop fully unrolled, so the row stays in registers instead of being stored and reloaded every
  step (15×15 multiply 3.6× faster). The row loop is unrolled only up to 8 rows; the unroll count is a
  template parameter because GCC accepts no expression there.
- **Gauss-Jordan reads a local copy of the pivot row.** The compiler then needs no proof that the
  updated row and the pivot row differ, and full-row updates give constant trip counts (15×15 inverse
  1.7× faster on GCC, 1.3× on Clang).
- **Fused multiply-add accumulation** throughout; requires hardware FMA (`x86-64-v3` / `armv8.2-a`).

Ruled out: `#pragma omp simd` (not allowed in `constexpr` functions) and
`-fvect-cost-model=dynamic` (a build-wide flag that fixes only part of the gap).

Known gaps: `-O3` (Release) makes GCC's 4×4 kernels 3–5× slower; GCC does not vectorise caller loops such
as `batch_transform_10k/f32`; the 4×4 inverse is ~2× behind Eigen's hand-written SIMD kernel.
