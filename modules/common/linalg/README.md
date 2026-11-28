# README: linalg

## Brief

A simple linear algebra library for robotic applications

- [Design notes](./docs/design.md) 
- [Worked example](./examples/example.cpp)
- [Benchmarks](./docs/benchmarking/benchmarking.md)

## Design considerations

- Header-only
- Fully `constexpr`: Results are provable at compile time with `static_assert`
- Exploits modern C++ and hardware features for performance on X86 and aarch 64 bit architectures
- No third party dependencies
- Scope limited to maths required for rigid body kinematics/dynamics in robotics and 3D graphics applications
  - Matrices and vectors
  - Quaternions and rotation matrices
  - Homogeneous transformations
- Coordinate frame-aware for compile-time safe coordinate transformations
- Numerically precise: fused multiply-adds, cancellation-safe cross products, pivoted elimination 
- Unambiguous APIs: Use of `Nan` and `std::nullopt` to report invalid results
- Interoperable with third party libraries such as glm and Eigen. Elements stored contiguously, so adapters are trivial

## Status

- [X] constexpr `Matrix` and `Vector` classes and supporting free functions
- [X] `std::format`/`std::println` formatter
- [X] Quaternion class and supporting free functions
- [ ] Numerically stable estimation (AHRS, EKF, nonlinear least squares)
  - [ ] Cholesky (LLT/LDLT) and `solve()`
  - [ ] QR for least squares
  - [ ] SO(3) `exp()`/`log()` and `skew()`, with small-angle branches
  - [ ] `symmetrize()` and Joseph-form covariance update
  - [ ] Range-safe `stableNorm()`
- [ ] Tagged `Attitude<To, From>` type as a thin wrapper over Quaternion
- [ ] [Automatic differentiation](https://solidean.com/blog/2026/fwd-diff-autodiff-cpp/)
- [ ] Use `std::linalg` backend when available
