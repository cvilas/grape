# AHRS Implementation Blueprint (ESKF/MEKF, Quaternion, 9-DoF IMU)

> This document uses LaTeX math. For best preview rendering in VS Code, open the Markdown preview with a math-enabled extension such as Markdown Preview Enhanced or a KaTeX/MathJax renderer.

---

## State Definition

Use a **nominal state + error-state** formulation.

### Nominal state
$$
\mathbf{x} =
\begin{bmatrix}
\mathbf{q}_{nb} \\
\mathbf{b}_g \\
\mathbf{b}_a 
\end{bmatrix}
$$
Where:
- $\mathbf{q}_{nb}$: quaternion rotating body $\to$ navigation (or inverse, just be consistent)
- $\mathbf{b}_g \in \mathbb{R}^3$: gyro bias
- $\mathbf{b}_a \in \mathbb{R}^3$: accel bias (optional in AHRS-only; recommended if long runtime)

### Error-state
$$
\delta \mathbf{x}=
\begin{bmatrix}
\delta \boldsymbol{\theta} \\
\delta \mathbf{b}_g \\
\delta \mathbf{b}_a
\end{bmatrix}
\in \mathbb{R}^{9}
$$

---

## 2) Continuous-Time Process Model

Given measurements:
- gyro: $\boldsymbol{\omega}_m$
- accel: $\mathbf{a}_m$
- mag: $\mathbf{m}_m$

### Gyro model
$$
\boldsymbol{\omega} = \boldsymbol{\omega}_m - \mathbf{b}_g - \mathbf{n}_g
$$

Quaternion kinematics (one common convention):
$$
\dot{\mathbf{q}} = \frac{1}{2}\,\Omega(\boldsymbol{\omega})\,\mathbf{q}
$$

Bias random walk:
$$
\dot{\mathbf{b}}_g = \mathbf{n}_{wg}, \quad
\dot{\mathbf{b}}_a = \mathbf{n}_{wa}
$$

---

## 3) Discrete Propagation

At each IMU sample ($\Delta t$):

1. Bias-corrected angular rate:
$$
\boldsymbol{\omega}_c = \boldsymbol{\omega}_m - \mathbf{b}_g
$$

2. Quaternion increment:
- small-angle: $\delta\boldsymbol{\theta} = \boldsymbol{\omega}_c \Delta t$
- incremental quaternion:
$$
\delta q \approx
\begin{bmatrix}
1 \\
\frac{1}{2}\delta\boldsymbol{\theta}
\end{bmatrix}
$$
(or exact exp map for larger angles)

3. Update:
$$
q_{k+1} = q_k \otimes \delta q
$$
and normalize.

4. Biases unchanged in nominal propagation (unless modeling deterministic drift).

---

## 4) Error-State Linearization

Standard small-angle form:
$$
\delta \dot{\boldsymbol{\theta}} = -[\boldsymbol{\omega}_c]_{\times} \delta\boldsymbol{\theta}
- \delta \mathbf{b}_g - \mathbf{n}_g
$$
$$
\delta \dot{\mathbf{b}}_g = \mathbf{n}_{wg}, \quad
\delta \dot{\mathbf{b}}_a = \mathbf{n}_{wa}
$$

So:
$$
\delta \dot{\mathbf{x}} = F \delta\mathbf{x} + G\mathbf{w}
$$

with (9-state):
$$
F=
\begin{bmatrix}
-[\omega_c]_{\times} & -I_3 & 0 \\
0 & 0 & 0 \\
0 & 0 & 0
\end{bmatrix}
$$

$$
G=
\begin{bmatrix}
-I_3 & 0 & 0 \\
0 & I_3 & 0 \\
0 & 0 & I_3
\end{bmatrix}
$$
if $\mathbf{w}=[n_g, n_{wg}, n_{wa}]^\top$.

Discretize:
$$
\Phi \approx I + F\Delta t
Where:
$$
$$
Q_d \approx G Q_c G^\top \Delta t
$$
Then:
$$
P_{k+1} = \Phi P_k \Phi^\top + Q_d
$$

---

## 5) Accelerometer Measurement Update (Tilt)

For attitude-only correction, model gravity direction:
$$
\hat{\mathbf{a}} = R_{bn}\,\mathbf{g}_n
$$
(measured in body frame; pick consistent frame direction/sign)

Residual:
$$
\mathbf{r}_a = \mathbf{a}_{unit} - \hat{\mathbf{a}}_{unit}
$$

Linearized Jacobian wrt $\delta\theta$:
$$
H_a = \begin{bmatrix}
-[\hat{\mathbf{a}}]_{\times} & 0 & 0
\end{bmatrix}
$$
(adjust sign based on your perturbation convention)

Use only when accel is reliable:
- $|\|\mathbf{a}_m\|-g| < \epsilon_a$, or
- adaptive $R_a$ inflation based on $|\|\mathbf{a}_m\|-g|$

---

## 6) Magnetometer Update (Heading)

### Calibration first (mandatory)
Apply:
$$
\mathbf{m}_{cal} = S(\mathbf{m}_m - \mathbf{b}_m)
$$
- $\mathbf{b}_m$: hard-iron offset
- $S$: soft-iron correction matrix

### Robust measurement strategy
Prefer heading-constraining update (horizontal field) in disturbed environments.

Residual form (vector):
$$
\mathbf{r}_m = \mathbf{m}_{unit} - \hat{\mathbf{m}}_{unit}, \quad
\hat{\mathbf{m}} = R_{bn}\,\mathbf{m}_n
$$
Jacobian analogous to accel:
$$
H_m = \begin{bmatrix}
-[\hat{\mathbf{m}}]_{\times} & 0 & 0
\end{bmatrix}
$$

### Disturbance handling
- innovation gating (chi-square)
- dynamic $R_m$ inflation near motors/current spikes
- reject update if field magnitude deviates from expected local norm

---

## 7) EKF Update Equations

Given $r, H, R$:
$$
S = HPH^\top + R
$$
$$
K = PH^\top S^{-1}
$$
$$
\delta x = K r
$$
$$
P \leftarrow (I-KH)P(I-KH)^\top + KRK^\top
$$
(Joseph form recommended)

### Injection
$$
\delta\theta = \delta x_{0:3}
$$
$$
q \leftarrow q \otimes \delta q(\delta\theta), \quad
\delta q \approx [1,\frac{1}{2}\delta\theta]
$$
$$
b_g \leftarrow b_g + \delta b_g,\quad
b_a \leftarrow b_a + \delta b_a
$$
Reset error-state mean to zero; apply corresponding reset Jacobian to $P$ if used.

---

## 8) Gating / Fault Logic (Production-Critical)

For each sensor update:
1. Compute NIS:
$$
\text{NIS} = r^\top S^{-1} r
$$
2. Compare with chi-square threshold:
- dof=3 vector update: e.g. 95% $\approx 7.815$, 99% $\approx 11.345$
3. If failed:
- reject update or inflate $R$
- increment fault counters
4. Recovery:
- re-enable after sustained good innovations

---

## 9) Tuning Seeds (Cheap MEMS Starting Point)

These are **starting points**, not final values.

- gyro noise density: $0.01 \sim 0.05\ \mathrm{rad/s/\sqrt{Hz}}$
- gyro bias RW: $1e{-5} \sim 1e{-3}\ \mathrm{rad/s^2/\sqrt{Hz}}$
- accel measurement sigma (normalized update): $0.02 \sim 0.1\ g$-equivalent
- mag measurement sigma (normalized): start loose; tighten after calibration
- accel gate stricter in static, looser in dynamic flight

Workflow:
1. tune gyro process first (drift vs noise)
2. tune accel update for tilt stability under vibration
3. tune mag update for heading without twitching near EMI
4. validate with logs and innovation statistics

---

## 10) Calibration Pipeline

1. **Gyro bias** at startup static average.
2. **Accel scale/misalignment** (6-position or better).
3. **Mag hard/soft iron** ellipsoid fit, plus frame alignment.
4. **Sensor-to-body rotation** (critical for UAV control quality).
5. Temperature compensation if available.

---

## 11) Timing / Numerical Practices

- use monotonic timestamps
- compute measured $\Delta t$ each sample
- clamp pathological dt spikes
- renormalize quaternion every step
- keep $P$ symmetric: $P = 0.5(P+P^\top)$
- check PSD (or add tiny diagonal jitter when needed)
- prefer double precision for filter core on Pi

---

## 12) C++ Module Layout Suggestion

- `ahrs/state.hpp` (state structs, quaternion utilities)
- `ahrs/eskf.hpp/.cpp` (predict/update core)
- `ahrs/models.hpp` (measurement models + Jacobians)
- `ahrs/gating.hpp` (NIS tests, fault counters)
- `ahrs/calibration.hpp` (mag/imu calibration application)
- `ahrs/config.hpp` (noise params, thresholds)
- `ahrs/logging.hpp` (innovations, NIS, flags)

Unit tests:
- Jacobian finite-difference checks
- quaternion injection/reset invariants
- covariance symmetry/PSD regression tests

---

## 13) Validation Plan

Bench:
- static bias convergence
- known rotations (hand-turn yaw/pitch/roll)
- magnetic disturbance tests (bring magnet nearby)

Robot/UAV:
- vibration tests with motors on
- aggressive yaw maneuvers
- repeated takeoff/landing heading repeatability
- long-duration drift runs without aiding

Metrics:
- attitude RMS vs reference (if available)
- heading drift rate
- innovation/NIS distributions
- update rejection rate by sensor type

---

