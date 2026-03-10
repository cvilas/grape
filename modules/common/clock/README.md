# README: Clock

## Brief

- `ClockBroadcaster`: Broadcasts timing ticks to listeners (`FollowerClock`) on the same host
- `FollowerClock`: Provides an interface similar to `std::chrono` clocks, but driven by ticks from the broadcaster  
