//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================
//
// Performance benchmarks for grape::linalg, optionally paired with Eigen. See
// ../docs/benchmarking.md for how to run, compare and track results.
//
// Benchmarks are named <operation>/<size>/<scalar>/<library> so that pairs sort together and
// either library can be selected with --benchmark_filter (e.g. '/linalg$' or 'multiply/').
//
// Methodology notes:
// - linalg is constexpr and header-only, so constant inputs would let the compiler evaluate a
//   benchmark at build time. Inputs are therefore random (seeded, for repeatability), generated at
//   runtime, and passed through benchmark::DoNotOptimize on every iteration.
// - Most benchmarks measure one independent operation per iteration (throughput-like). The gyro
//   integration benchmark feeds each result into the next iteration, measuring latency.
// - Eigen benchmarks receive the same input values as their linalg counterparts. Conventions
//   differ (Eigen quaternions are Hamilton, linalg's are JPL), which does not affect cost.

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>
#include <random>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#include "grape/linalg/frames.h"
#include "grape/linalg/matrix.h"
#include "grape/linalg/quaternion.h"

#ifdef GRAPE_LINALG_BENCH_EIGEN
#include <Eigen/Dense>
#include <Eigen/Geometry>
#endif

namespace {

namespace la = grape::linalg;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

//=================================================================================================
// Inputs
//=================================================================================================

constexpr auto RANDOM_SEED = 42U;
constexpr auto BATCH_SIZE = 10'000UZ;

struct World : la::Frame {};
struct Body : la::Frame {};
struct Point : la::PointTag {};

//-------------------------------------------------------------------------------------------------
auto generator() -> std::mt19937& {
  // A fixed seed is deliberate: every run benchmarks the same inputs
  // NOLINTNEXTLINE(bugprone-random-generator-seed,cert-msc32-c,cert-msc51-cpp)
  static auto gen = std::mt19937{ RANDOM_SEED };
  return gen;
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
auto uniform() -> Scalar {
  auto dist = std::uniform_real_distribution<Scalar>{ Scalar{ -1 }, Scalar{ 1 } };
  return dist(generator());
}

//-------------------------------------------------------------------------------------------------
template <std::size_t NumRows, std::size_t NumColumns, std::floating_point Scalar>
auto randomMatrix() -> la::Matrix<NumRows, NumColumns, Scalar> {
  auto mat = la::Matrix<NumRows, NumColumns, Scalar>{};
  std::ranges::generate(mat.elements(), uniform<Scalar>);
  return mat;
}

//-------------------------------------------------------------------------------------------------
/// Diagonally dominant, hence comfortably invertible
template <std::size_t Order, std::floating_point Scalar>
auto invertibleMatrix() -> la::Matrix<Order, Order, Scalar> {
  return randomMatrix<Order, Order, Scalar>() +
         (la::identity<Order, Scalar>() * static_cast<Scalar>(Order));
}

//-------------------------------------------------------------------------------------------------
/// Symmetric positive definite, as covariance matrices are
template <std::size_t Order, std::floating_point Scalar>
auto covarianceMatrix(Scalar scale) -> la::Matrix<Order, Order, Scalar> {
  const auto mat = randomMatrix<Order, Order, Scalar>();
  return ((mat * la::transpose(mat)) + la::identity<Order, Scalar>()) * scale;
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
auto randomQuaternion() -> la::Quaternion<Scalar> {
  return la::normalized(la::Quaternion<Scalar>{
      .x = uniform<Scalar>(),
      .y = uniform<Scalar>(),
      .z = uniform<Scalar>(),
      .w = uniform<Scalar>(),
  });
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
auto randomTransform() -> la::Transform<World, Body, Scalar> {
  return la::Transform<World, Body, Scalar>{
    la::Rotation<World, Body, Scalar>::fromQuaternion(randomQuaternion<Scalar>()),
    la::Displacement<World, World, Body, Scalar>{ randomMatrix<3, 1, Scalar>() },
  };
}

//-------------------------------------------------------------------------------------------------
template <typename Scalar>
constexpr auto scalarName() -> const char* {
  return std::same_as<Scalar, float> ? "f32" : "f64";
}

//-------------------------------------------------------------------------------------------------
/// One step of a 15-state error-state Kalman filter with a 6-dimensional measurement, the shape
/// of a typical IMU/INS estimator
template <std::floating_point Scalar>
struct KalmanProblem {
  static constexpr auto NUM_STATES = 15UZ;
  static constexpr auto NUM_MEASUREMENTS = 6UZ;

  using StateMatrix = la::Matrix<NUM_STATES, NUM_STATES, Scalar>;
  using MeasurementMatrix = la::Matrix<NUM_MEASUREMENTS, NUM_MEASUREMENTS, Scalar>;

  la::Vector<NUM_STATES, Scalar> state = randomMatrix<NUM_STATES, 1, Scalar>();
  StateMatrix covariance = covarianceMatrix<NUM_STATES, Scalar>(static_cast<Scalar>(0.1));
  StateMatrix transition =
      la::identity<NUM_STATES, Scalar>() +
      (randomMatrix<NUM_STATES, NUM_STATES, Scalar>() * static_cast<Scalar>(0.01));
  StateMatrix process_noise = covarianceMatrix<NUM_STATES, Scalar>(static_cast<Scalar>(0.001));
  la::Matrix<NUM_MEASUREMENTS, NUM_STATES, Scalar> observation =
      randomMatrix<NUM_MEASUREMENTS, NUM_STATES, Scalar>();
  MeasurementMatrix measurement_noise =
      covarianceMatrix<NUM_MEASUREMENTS, Scalar>(static_cast<Scalar>(0.01));
  la::Vector<NUM_MEASUREMENTS, Scalar> measurement = randomMatrix<NUM_MEASUREMENTS, 1, Scalar>();
};

//=================================================================================================
// linalg benchmarks
//=================================================================================================

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void linalgMultiply(benchmark::State& state) {
  auto lhs = randomMatrix<Order, Order, Scalar>();
  auto rhs = randomMatrix<Order, Order, Scalar>();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(lhs);
    benchmark::DoNotOptimize(rhs);
    auto product = lhs * rhs;
    benchmark::DoNotOptimize(product);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void linalgDeterminant(benchmark::State& state) {
  auto mat = randomMatrix<Order, Order, Scalar>();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(mat);
    auto det = la::determinant(mat);
    benchmark::DoNotOptimize(det);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void linalgInverse(benchmark::State& state) {
  auto mat = invertibleMatrix<Order, Scalar>();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(mat);
    auto inv = la::inverse(mat);
    benchmark::DoNotOptimize(inv);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void linalgQuaternionMultiply(benchmark::State& state) {
  auto lhs = randomQuaternion<Scalar>();
  auto rhs = randomQuaternion<Scalar>();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(lhs);
    benchmark::DoNotOptimize(rhs);
    auto product = lhs * rhs;
    benchmark::DoNotOptimize(product);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void linalgQuaternionFromMatrix(benchmark::State& state) {
  auto mat = la::rotationMatrix(randomQuaternion<Scalar>());
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(mat);
    auto quat = la::Quaternion<Scalar>::fromMatrix(mat);
    benchmark::DoNotOptimize(quat);
  }
}

//-------------------------------------------------------------------------------------------------
/// Latency of a dependent chain: each step consumes the previous result
template <std::floating_point Scalar>
void linalgGyroIntegration(benchmark::State& state) {
  auto attitude = randomQuaternion<Scalar>();
  auto increment = la::Quaternion<Scalar>::fromAxisAngle(
      la::Vector<3, Scalar>{ Scalar{ 0 }, Scalar{ 0 }, Scalar{ 1 } }, static_cast<Scalar>(0.001));
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(increment);
    attitude = la::normalized(attitude * increment);
    benchmark::DoNotOptimize(attitude);
  }
}

//-------------------------------------------------------------------------------------------------
/// Frame-typed transform of a position. Must cost the same as linalgRawTransformPosition: the
/// frame types are meant to be free.
template <std::floating_point Scalar>
void linalgTransformPosition(benchmark::State& state) {
  auto transform = randomTransform<Scalar>();
  auto position = la::Position<Body, Point, Scalar>{ randomMatrix<3, 1, Scalar>() };
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(transform);
    benchmark::DoNotOptimize(position);
    auto mapped = transform * position;
    benchmark::DoNotOptimize(mapped);
  }
}

//-------------------------------------------------------------------------------------------------
/// The same arithmetic as linalgTransformPosition on untyped matrices
template <std::floating_point Scalar>
void linalgRawTransformPosition(benchmark::State& state) {
  const auto transform = randomTransform<Scalar>();
  auto rotation = transform.rotation().matrix();
  auto translation = transform.translation().coordinates();
  auto position = randomMatrix<3, 1, Scalar>();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(rotation);
    benchmark::DoNotOptimize(translation);
    benchmark::DoNotOptimize(position);
    auto mapped = translation;
    for (std::size_t col = 0UZ; col < 3UZ; ++col) {
      for (std::size_t row = 0UZ; row < 3UZ; ++row) {
        mapped(row) = la::detail::multiplyAdd(rotation(row, col), position(col), mapped(row));
      }
    }
    benchmark::DoNotOptimize(mapped);
  }
}

//-------------------------------------------------------------------------------------------------
/// Throughput of transforming a batch of points (e.g. a point cloud or splat positions)
template <std::floating_point Scalar>
void linalgBatchTransform(benchmark::State& state) {
  auto transform = randomTransform<Scalar>();
  auto input = std::vector<la::Position<Body, Point, Scalar>>(BATCH_SIZE);
  std::ranges::generate(
      input, [] { return la::Position<Body, Point, Scalar>{ randomMatrix<3, 1, Scalar>() }; });
  auto output = std::vector<la::Position<World, Point, Scalar>>(BATCH_SIZE);
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(transform);
    // escape the output buffer so its stores cannot be dropped as dead
    benchmark::DoNotOptimize(output.data());
    std::ranges::transform(input, output.begin(),
                           [&transform](const auto& point) { return transform * point; });
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(BATCH_SIZE));
}

//-------------------------------------------------------------------------------------------------
/// One predict + update step of a 15-state Kalman filter, inverting the innovation covariance
template <std::floating_point Scalar>
void linalgKalmanStep(benchmark::State& state) {
  using Problem = KalmanProblem<Scalar>;
  auto problem = Problem{};
  const auto ident = la::identity<Problem::NUM_STATES, Scalar>();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(problem);
    const auto& tr = problem.transition;
    const auto& obs = problem.observation;
    // predict
    auto state_est = tr * problem.state;
    auto cov = (tr * problem.covariance * la::transpose(tr)) + problem.process_noise;
    // update
    const auto innovation = problem.measurement - (obs * state_est);
    const auto cov_obs_t = cov * la::transpose(obs);
    const auto innovation_cov = (obs * cov_obs_t) + problem.measurement_noise;
    const auto innovation_cov_inv = la::inverse(innovation_cov);
    if (!innovation_cov_inv.has_value()) {
      state.SkipWithError("singular innovation covariance");
      break;
    }
    const auto gain = cov_obs_t * innovation_cov_inv.value_or(innovation_cov);
    state_est = state_est + (gain * innovation);
    cov = (ident - (gain * obs)) * cov;
    benchmark::DoNotOptimize(state_est);
    benchmark::DoNotOptimize(cov);
  }
}

#ifdef GRAPE_LINALG_BENCH_EIGEN
//=================================================================================================
// Eigen counterparts
//=================================================================================================

//-------------------------------------------------------------------------------------------------
/// @return Eigen copy of a linalg matrix, read through linalg's contiguous row-major storage
template <std::size_t NumRows, std::size_t NumColumns, typename Scalar>
auto toEigen(const la::Matrix<NumRows, NumColumns, Scalar>& mat)
    -> Eigen::Matrix<Scalar, static_cast<int>(NumRows), static_cast<int>(NumColumns)> {
  constexpr auto LAYOUT = (NumColumns == 1UZ) ? Eigen::ColMajor : Eigen::RowMajor;
  using RowMajorMap = Eigen::Map<
      const Eigen::Matrix<Scalar, static_cast<int>(NumRows), static_cast<int>(NumColumns), LAYOUT>>;
  return RowMajorMap(mat.elements().data());
}

//-------------------------------------------------------------------------------------------------
template <typename Scalar>
auto toEigen(const la::Quaternion<Scalar>& quat) -> Eigen::Quaternion<Scalar> {
  return Eigen::Quaternion<Scalar>{ quat.w, quat.x, quat.y, quat.z };
}

//-------------------------------------------------------------------------------------------------
template <typename Scalar>
auto toEigen(const la::Transform<World, Body, Scalar>& transform)
    -> Eigen::Transform<Scalar, 3, Eigen::Isometry> {
  auto result = Eigen::Transform<Scalar, 3, Eigen::Isometry>::Identity();
  result.linear() = toEigen(transform.rotation().matrix());
  result.translation() = toEigen(transform.translation().coordinates());
  return result;
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void eigenMultiply(benchmark::State& state) {
  auto lhs = toEigen(randomMatrix<Order, Order, Scalar>());
  auto rhs = toEigen(randomMatrix<Order, Order, Scalar>());
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(lhs);
    benchmark::DoNotOptimize(rhs);
    auto product = (lhs * rhs).eval();
    benchmark::DoNotOptimize(product);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void eigenDeterminant(benchmark::State& state) {
  auto mat = toEigen(randomMatrix<Order, Order, Scalar>());
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(mat);
    auto det = mat.determinant();
    benchmark::DoNotOptimize(det);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void eigenInverse(benchmark::State& state) {
  auto mat = toEigen(invertibleMatrix<Order, Scalar>());
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(mat);
    auto inv = mat.inverse().eval();
    benchmark::DoNotOptimize(inv);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void eigenQuaternionMultiply(benchmark::State& state) {
  auto lhs = toEigen(randomQuaternion<Scalar>());
  auto rhs = toEigen(randomQuaternion<Scalar>());
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(lhs);
    benchmark::DoNotOptimize(rhs);
    auto product = lhs * rhs;
    benchmark::DoNotOptimize(product);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void eigenQuaternionFromMatrix(benchmark::State& state) {
  auto mat = toEigen(la::rotationMatrix(randomQuaternion<Scalar>()));
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(mat);
    auto quat = Eigen::Quaternion<Scalar>{ mat };
    benchmark::DoNotOptimize(quat);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void eigenGyroIntegration(benchmark::State& state) {
  auto attitude = toEigen(randomQuaternion<Scalar>());
  const auto axis_angle =
      Eigen::AngleAxis<Scalar>(static_cast<Scalar>(0.001), Eigen::Matrix<Scalar, 3, 1>::UnitZ());
  auto increment = Eigen::Quaternion<Scalar>(axis_angle);
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(increment);
    attitude = (attitude * increment).normalized();
    benchmark::DoNotOptimize(attitude);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void eigenTransformPosition(benchmark::State& state) {
  auto transform = toEigen(randomTransform<Scalar>());
  auto position = toEigen(randomMatrix<3, 1, Scalar>());
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(transform);
    benchmark::DoNotOptimize(position);
    auto mapped = (transform * position).eval();
    benchmark::DoNotOptimize(mapped);
  }
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void eigenBatchTransform(benchmark::State& state) {
  using Vec3 = Eigen::Matrix<Scalar, 3, 1>;
  auto transform = toEigen(randomTransform<Scalar>());
  auto input = std::vector<Vec3>(BATCH_SIZE);
  std::ranges::generate(input, [] { return toEigen(randomMatrix<3, 1, Scalar>()); });
  auto output = std::vector<Vec3>(BATCH_SIZE);
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(transform);
    // escape the output buffer so its stores cannot be dropped as dead
    benchmark::DoNotOptimize(output.data());
    std::ranges::transform(input, output.begin(),
                           [&transform](const Vec3& point) -> Vec3 { return transform * point; });
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(BATCH_SIZE));
}

//-------------------------------------------------------------------------------------------------
/// Kalman step with the innovation covariance inverted, matching linalgKalmanStep
template <std::floating_point Scalar>
void eigenKalmanStepInverse(benchmark::State& state) {
  using Problem = KalmanProblem<Scalar>;
  constexpr auto NUM_STATES = static_cast<int>(Problem::NUM_STATES);
  const auto source = Problem{};
  auto state_vec = toEigen(source.state);
  auto covariance = toEigen(source.covariance);
  const auto tr = toEigen(source.transition);
  const auto process_noise = toEigen(source.process_noise);
  const auto obs = toEigen(source.observation);
  const auto measurement_noise = toEigen(source.measurement_noise);
  const auto measurement = toEigen(source.measurement);
  const auto ident = Eigen::Matrix<Scalar, NUM_STATES, NUM_STATES>::Identity();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(state_vec);
    benchmark::DoNotOptimize(covariance);
    auto state_est = (tr * state_vec).eval();
    auto cov = ((tr * covariance * tr.transpose()) + process_noise).eval();
    const auto innovation = (measurement - (obs * state_est)).eval();
    const auto cov_obs_t = (cov * obs.transpose()).eval();
    const auto innovation_cov = ((obs * cov_obs_t) + measurement_noise).eval();
    const auto gain = (cov_obs_t * innovation_cov.inverse()).eval();
    state_est += gain * innovation;
    cov = ((ident - (gain * obs)) * cov).eval();
    benchmark::DoNotOptimize(state_est);
    benchmark::DoNotOptimize(cov);
  }
}

//-------------------------------------------------------------------------------------------------
/// Idiomatic Eigen Kalman step: solves with an LDLT factorisation instead of inverting. Shows what
/// linalg would gain from a symmetric solver.
template <std::floating_point Scalar>
void eigenKalmanStepLdlt(benchmark::State& state) {
  using Problem = KalmanProblem<Scalar>;
  constexpr auto NUM_STATES = static_cast<int>(Problem::NUM_STATES);
  const auto source = Problem{};
  auto state_vec = toEigen(source.state);
  auto covariance = toEigen(source.covariance);
  const auto tr = toEigen(source.transition);
  const auto process_noise = toEigen(source.process_noise);
  const auto obs = toEigen(source.observation);
  const auto measurement_noise = toEigen(source.measurement_noise);
  const auto measurement = toEigen(source.measurement);
  const auto ident = Eigen::Matrix<Scalar, NUM_STATES, NUM_STATES>::Identity();
  for (const auto unused : state) {
    (void)unused;
    benchmark::DoNotOptimize(state_vec);
    benchmark::DoNotOptimize(covariance);
    auto state_est = (tr * state_vec).eval();
    auto cov = ((tr * covariance * tr.transpose()) + process_noise).eval();
    const auto innovation = (measurement - (obs * state_est)).eval();
    const auto obs_cov = (obs * cov).eval();
    const auto innovation_cov = ((obs_cov * obs.transpose()) + measurement_noise).eval();
    // K = P H^T S^-1  <=>  K^T = S^-1 H P, since S and P are symmetric
    const auto gain = innovation_cov.ldlt().solve(obs_cov).transpose().eval();
    state_est += gain * innovation;
    cov = ((ident - (gain * obs)) * cov).eval();
    benchmark::DoNotOptimize(state_est);
    benchmark::DoNotOptimize(cov);
  }
}
#endif

//=================================================================================================
// Registration
//=================================================================================================

//-------------------------------------------------------------------------------------------------
void add(const std::string& name, void (*func)(benchmark::State&)) {
  benchmark::RegisterBenchmark(name, func);
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, typename Scalar>
auto sizeName() -> std::string {
  return std::format("{}x{}/{}", Order, Order, scalarName<Scalar>());
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void registerMultiply() {
  add("multiply/" + sizeName<Order, Scalar>() + "/linalg", linalgMultiply<Order, Scalar>);
#ifdef GRAPE_LINALG_BENCH_EIGEN
  add("multiply/" + sizeName<Order, Scalar>() + "/eigen", eigenMultiply<Order, Scalar>);
#endif
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void registerDeterminant() {
  add("determinant/" + sizeName<Order, Scalar>() + "/linalg", linalgDeterminant<Order, Scalar>);
#ifdef GRAPE_LINALG_BENCH_EIGEN
  add("determinant/" + sizeName<Order, Scalar>() + "/eigen", eigenDeterminant<Order, Scalar>);
#endif
}

//-------------------------------------------------------------------------------------------------
template <std::size_t Order, std::floating_point Scalar>
void registerInverse() {
  add("inverse/" + sizeName<Order, Scalar>() + "/linalg", linalgInverse<Order, Scalar>);
#ifdef GRAPE_LINALG_BENCH_EIGEN
  add("inverse/" + sizeName<Order, Scalar>() + "/eigen", eigenInverse<Order, Scalar>);
#endif
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void registerPerScalar() {
  const auto suffix = std::string{ "/" } + scalarName<Scalar>();
  add("quaternion_multiply" + suffix + "/linalg", linalgQuaternionMultiply<Scalar>);
  add("quaternion_from_matrix" + suffix + "/linalg", linalgQuaternionFromMatrix<Scalar>);
  add("gyro_integration_latency" + suffix + "/linalg", linalgGyroIntegration<Scalar>);
  add("transform_position" + suffix + "/linalg", linalgTransformPosition<Scalar>);
  add("transform_position" + suffix + "/linalg_raw", linalgRawTransformPosition<Scalar>);
  add("batch_transform_10k" + suffix + "/linalg", linalgBatchTransform<Scalar>);
  add("kalman_step_15x6" + suffix + "/linalg", linalgKalmanStep<Scalar>);
#ifdef GRAPE_LINALG_BENCH_EIGEN
  add("quaternion_multiply" + suffix + "/eigen", eigenQuaternionMultiply<Scalar>);
  add("quaternion_from_matrix" + suffix + "/eigen", eigenQuaternionFromMatrix<Scalar>);
  add("gyro_integration_latency" + suffix + "/eigen", eigenGyroIntegration<Scalar>);
  add("transform_position" + suffix + "/eigen", eigenTransformPosition<Scalar>);
  add("batch_transform_10k" + suffix + "/eigen", eigenBatchTransform<Scalar>);
  add("kalman_step_15x6" + suffix + "/eigen", eigenKalmanStepInverse<Scalar>);
  add("kalman_step_15x6" + suffix + "/eigen_ldlt", eigenKalmanStepLdlt<Scalar>);
#endif
}

//-------------------------------------------------------------------------------------------------
template <std::floating_point Scalar>
void registerAll() {
  registerMultiply<3, Scalar>();
  registerMultiply<4, Scalar>();
  registerMultiply<6, Scalar>();
  registerDeterminant<3, Scalar>();
  registerDeterminant<4, Scalar>();
  registerDeterminant<15, Scalar>();
  registerInverse<4, Scalar>();
  registerInverse<15, Scalar>();
  registerPerScalar<Scalar>();
}

//-------------------------------------------------------------------------------------------------
/// Records what produced the results, so stored results stay interpretable
void addContext() {
  benchmark::AddCustomContext("compiler", __VERSION__);
  benchmark::AddCustomContext("target_arch", GRAPE_TARGET_ARCH);
#ifdef GRAPE_LINALG_BENCH_EIGEN
#ifdef EIGEN_VERSION_STRING
  benchmark::AddCustomContext("eigen", EIGEN_VERSION_STRING);
#else
  benchmark::AddCustomContext("eigen", std::format("{}.{}.{}", EIGEN_WORLD_VERSION,
                                                   EIGEN_MAJOR_VERSION, EIGEN_MINOR_VERSION));
#endif
#else
  benchmark::AddCustomContext("eigen", "not built");
#endif
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace

//-------------------------------------------------------------------------------------------------
auto main(int argc, char** argv) -> int {
  try {
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
      return EXIT_FAILURE;
    }
    addContext();
    registerAll<float>();
    registerAll<double>();
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    (void)std::fputs(ex.what(), stderr);  // C I/O cannot throw, unlike std::println
    return EXIT_FAILURE;
  } catch (...) {
    return EXIT_FAILURE;
  }
}
