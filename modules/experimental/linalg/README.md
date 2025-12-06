# README: linalg

## Brief

A simple linear algebra library for robotic applications

## Detailed description

- [ ] Matrix classes for robot math
- [ ] Coordinate frame [awareness](https://github.com/cvilas/scratch/blob/master/linalg.cpp)
- [ ] linear algebra internal implementation using std::mdspan and c++26 linalg 
- [ ] Interoperable with glm

## TODO

- [ ] constexpr `Matrix<T,Nr,Nc>` class
- [ ] methods
  - [ ] get/set submatrix
- [ ] constexpr `Vector<T,N>` class defined in terms of Matrix
- [ ] Free functions
  - [ ] initialiser (comma separated values)
  - [ ] add
  - [ ] sub
  - [ ] scale
  - [ ] mult
  - [ ] trace
  - [ ] determinant
  - [ ] transpose
  - [ ] inverse
  - [ ] println formatter
  - [ ] dot
  - [ ] cross
- [ ] transform
  - [ ] rx, ry, rz
  - [ ] rotate
  - [ ] translate
  - [ ] invert
- [ ] Quaternion class and supporting free functions

## References

- Robotics at Compile Time: Optimizing Robotics Algorithms With C++'s Compile-Time Features - [CppCon23](https://youtu.be/Y6AUsB3RUhA) 
- Stellar at LSU: [CSC4700 - Linear Algebra in C++](https://youtu.be/XzUTLsWyErA)