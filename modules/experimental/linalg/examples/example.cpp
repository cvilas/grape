//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <numbers>
#include <print>
#include <tuple>

#include "grape/linalg/frames.h"
#include "grape/linalg/matrix.h"

//=================================================================================================
// Demonstrates the two layers of the linalg module:
//
// 1. Raw numerics: Matrix/Vector and free functions, here solving x in y = Ax + b
// 2. Frame-aware geometry: the same arithmetic with coordinate frames encoded in types, so that
//    mixing incompatible frames fails to compile. The scenario is a robot in a world frame
//    carrying a camera that observes a landmark.
//
// See docs/frames_and_numerics.md for the accompanying primer.
//=================================================================================================

namespace {

//-------------------------------------------------------------------------------------------------
auto solveLinearSystem() -> bool {
  using grape::linalg::inverse;
  using grape::linalg::Matrix;
  using grape::linalg::Vector;

  // y = Ax + b, solved for x as inverse(A) * (y - b)
  const auto a_mat = Matrix<3, 3>{ 2., 0., 1.,  //
                                   0., 4., 0.,  //
                                   0., 0., 8. };
  const auto b_vec = Vector<3>{ 1., 1., 1. };
  const auto y_vec = Vector<3>{ 4., 9., 25. };

  const auto a_inv = inverse(a_mat);
  if (not a_inv.has_value()) {
    std::println(stderr, "A = {} is singular", a_mat);
    return false;
  }
  const auto x_vec = (*a_inv) * (y_vec - b_vec);
  std::println("y = Ax + b with A = {}, b = {}, y = {}", a_mat, b_vec, y_vec);
  std::println("  => x = {:.3f}", x_vec);
  return true;
}

//-------------------------------------------------------------------------------------------------
void locateLandmark() {
  using grape::linalg::Displacement;
  using grape::linalg::Position;
  using grape::linalg::Rotation;
  using grape::linalg::Transform;

  // Tag types for the frames and points in the scenario. Tags occupy no storage; they only exist
  // to let the compiler check frame bookkeeping.
  struct World : grape::linalg::Frame {};
  struct Body : grape::linalg::Frame {};
  struct Camera : grape::linalg::Frame {};
  struct Landmark : grape::linalg::PointTag {};

  // Robot body: yawed +90 degrees and offset from the world origin
  const auto pose_wb =
      Transform<World, Body>{ Rotation<Body, World>::aboutZ(std::numbers::pi / 2).inverse(),
                              Displacement<World, World, Body>{ 1., 2., 0. } };

  // Camera: mounted 1m ahead of the body origin, aligned with the body
  const auto pose_bc = Transform<Body, Camera>{ Rotation<Body, Camera>{},
                                                Displacement<Body, Body, Camera>{ 1., 0., 0. } };

  // Transforms compose along the frame chain: T_WC = T_WB * T_BC
  const auto pose_wc = pose_wb * pose_bc;

  // The camera observes a landmark 1m ahead of it. Where is it in the world?
  const auto p_camera = Position<Camera, Landmark>{ 1., 0., 0. };
  const auto p_world = pose_wc * p_camera;
  std::println("Landmark at {:.3f} in camera frame is at {:.3f} in world frame",
               p_camera.coordinates(), p_world.coordinates());

  // And back again, using the inverse transform
  const auto p_recovered = pose_wc.inverse() * p_world;
  std::println("  => recovered in camera frame: {:.3f}", p_recovered.coordinates());

  // Frame mismatches do not compile. For instance:
  //   pose_wc * p_world;   // error: expects a Camera-relative position
  //   pose_bc * pose_wb;   // error: broken frame chain T_BC * T_WB
}

}  // namespace

//=================================================================================================
auto main() -> int {
  try {
    if (not solveLinearSystem()) {
      return EXIT_FAILURE;
    }
    locateLandmark();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    std::ignore = std::fputs("Unknown exception\n", stderr);
    return EXIT_FAILURE;
  }
}
