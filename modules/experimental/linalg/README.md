# README: linalg

## Brief

A simple linear algebra library for robotic applications

- [Primer: linear algebra for rigid body motion](./docs/frames_and_numerics.md) — an educational,
  first-principles walkthrough of the concepts and the code that implements them
- [Worked example](./examples/example.cpp)

## Design considerations

- Header-only, `constexpr` throughout: results are provable at compile time (see the
  `static_assert` checks in [tests](./tests/))
- Exploits modern C++ (C++26) and advanced features for performance on X86 and aarch 64 bit
  architectures
- No third party dependencies
- Designed for dynamics of rigid body motion for application in mobile robotics, drones, robotic
  manipulation and 3D graphics
  - quaternions and rotation matrices
  - rigid body transformations
  - matrices and vectors
- Coordinate frame aware for compile-time safe and accurate coordinate transformations and
  operations, following [Furgale](./docs/furgale_tutorial.pdf) conventions
- Numerically careful by construction: fused multiply-add accumulation, cancellation-safe cross
  products, pivoted elimination, singularity reported as `std::nullopt` rather than garbage
- Interoperable with glm and Eigen: elements are stored contiguously in row-major order, so
  adapters are trivial to add when a consumer appears

## Layout

| Header                                           | Contents                                                     |
|--------------------------------------------------|--------------------------------------------------------------|
| [`matrix.h`](./include/grape/linalg/matrix.h)    | `Matrix<Nr, Nc, T>`, `Vector<N, T>` and free functions       |
| [`quaternion.h`](./include/grape/linalg/quaternion.h) | `Quaternion<T>` algebra (Trawny conventions)           |
| [`frames.h`](./include/grape/linalg/frames.h)    | Frame-aware `Vector3`, `Displacement`, `Position`, `Rotation`, `Transform` |

## Status

- [X] constexpr `Matrix` and `Vector` classes and supporting free functions
- [X] `std::format`/`std::println` formatter
- [X] Quaternion class and supporting free functions
- [X] Compile-time coordinate-frame-aware rotations and rigid transforms (rotate, translate, compose, invert)
- [ ] Blocked/tiled multiplication tuned for large matrices
- [ ] [Automatic differentiation](https://solidean.com/blog/2026/fwd-diff-autodiff-cpp/)
- [ ] Use `std::linalg` backend when available