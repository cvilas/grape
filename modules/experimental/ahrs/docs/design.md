# AHRS design notes

## Background

- Goal: Robust attitude estimator using a cheap IMU (3-axis accel, gyro, magnetometer), capable of running on a raspberry Pi
- Algorithm: Error-State Extended Kalman Filter (ESKF / MEKF) on SO(3) with quaternion nominal state. Rationale:
  - principled gyro bias estimation
  - covariance-based sensor fusion
  - innovation gating and outlier rejection
  - adaptive sensor confidence tuning
  - straightforward extension to aided navigation (GNSS/vision/baro/odom)

## Filter Architecture

- Propagation (high rate): state includes quaternion and gyro bias (optionally accel bias); integrate angular rate after bias correction
- Error-state covariance propagation: small-angle attitude error model + bias random walk
- Accelerometer update: tilt correction with gating/adaptive downweighting under high specific force
- Magnetometer update: heading correction with strong anomaly handling, prefer horizontal/heading information when full-vector mag is unreliable
- Error injection/reset: inject small-angle correction into nominal quaternion and reset error-state
- Numerical hygiene: Quaternion renormalization, covariance symmetry/PSD maintenance

## Robustness Requirements Highlighted
- Hard-iron + soft-iron magnetometer calibration
- Sensor-to-body frame alignment calibration
- Innovation chi-square gating for accel/mag
- Adaptive measurement noise inflation under vibration/dynamics/magnetic interference
- Strict timestamp/dt handling (jitter-aware)
- Coning-aware integration for high-dynamic rotational motion

## References 

### Theory

- N. Trawny, S. I. Roumeliotis, *Indirect Kalman Filter for 3D Attitude Estimation*, Univ. of Minnesota, Dept. of CSE, TR-2005-002, 2005.
- Joan Solà, *Quaternion kinematics for the error-state Kalman filter*, arXiv:1711.02508, 2017, https://arxiv.org/abs/1711.02508
- F. Landis Markley, *Attitude Error Representations for Kalman Filtering*, Journal of Guidance, Control, and Dynamics, 26(2):311–317, 2003, DOI: 10.2514/2.5048

### Implementation

- PX4 sources, specifically 
  - `src/modules/ekf2/EKF/ekf.cpp`
  - `src/modules/ekf2/EKF/ekf.h`
  - `src/modules/ekf2/EKF/covariance.cpp`
  - `src/modules/ekf2/EKF/mag_control.cpp`
  - `src/modules/ekf2/EKF/mag_fusion.cpp`
  - `src/modules/ekf2/EKF/yaw_fusion.cpp` (or equivalent depending on PX4 version)
  - `src/modules/ekf2/EKF/control.cpp`
  - `src/modules/ekf2/EKF/estimator_status.cpp`
  - `src/modules/ekf2/EKF/common.h`
  - `src/modules/ekf2/ekf2_params.c` (or version-specific parameter metadata)
- What to look for in code
  - nominal quaternion + small-angle error-state pattern
  - process noise and bias random walk handling
  - innovation test ratios / gating logic
  - mag rejection/fallback and heading fusion modes
  - reset and estimator health logic
  - timing/downsampling handling and dt consistency

### Suggested Reading Order

- Solà + Trawny + Markley (conventions and theory)
- `ekf.h` / `ekf.cpp` (top-level flow)
- `covariance.cpp` (P propagation/update)
- mag/yaw fusion files (heading robustness)
- control/status files (fault handling)
- params/common definitions (tuning map)


