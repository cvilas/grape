# README: Ego Clock

## Brief

Provides a single unified means of time-keeping for components of a system distributed across a LAN.

EgoClock provides timing with reference to a clock that is custom/internal to the plant or process 
under control. The clock is driven by periodic ticks from EgoClockDriver, which in turn could be 
driven by a user-defined source, e.g. a real-time operating system, hardware clock, or physics 
simulation. Indeed, even an NTP synchronised clock can be used as the source, in which case it just 
mimics the behaviour of WallClock.

![Ego Clock](./docs/ego_clock.drawio.svg)

## Assumptions and constraints

- Subsystems may be distributed across a LAN
- All subsystems use ego-clock for timing and not std::chrono. 
- All participating hosts are NTP synchronised, as network (wall) time serves as the 
  global reference for timing calculations

## Usage

- Master clock calls `EgoClockDriver::tick` periodically. 
- All clocks in the system synchronise every _N_ ticks where `N=EgoClockDriver::Config::boardcast_interval` 
- Select `EgoClockDriver::Config` parameter based on heurisitcs. 
  - `broadcast_interval`: Higher = less network traffic, slower clock updates
  - `calibration_window`: Higher = smoother fit, slower response to drift
  - Tune for low RMSE at optimal update rate 

## TODO

- [ ] `EgoClock` reports stale clock transforms (include `valid_until` field in `ClockTransform`) 
