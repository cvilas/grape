# Probe design guidelines

## Definitions

- Plant: The real-time (RT) controlled process, likely running on an embedded processor  
- Controller: The component of Probe integrated with the Plant, and hooks to various Plant signals that need monitoring 
- Monitor: The supervisor that monitors the Plant through the controller, and running on a non-RT thread on the same processor or on a remote networked processor     
- Signal: A numeric quantity that is registered by the Controller for supervision from the Monitor. They are of two kinds
  - Watchable: A numeric quantity only registered for observation from the Monitor 
  - Controllable: A numeric quantity registered for both observation and manipulation from the Monitor
- Record: An aggregate sample of all Watchables and Controllables at a specific timestamp, including the timestamp

## Requirements

- Watchable data types
  - _Shall_ be time-series integer or floating-point data
  - _Should_ include time-series image data
  - _Should_ include time-series audio data
- Controllable data types 
  - _Shall_ be time-series integer or floating-point data
  - _Shall_ be initialisable from a configuration script loaded at runtime
- Controller 
  - _Shall_ provide mechanism to register Watchables that are available for remote observation from the Monitor
  - _Shall_ provide mechanism to register Controllables that are available for remote observation and manipulation from the Monitor
  - _Shall_ transport records to the Monitor in batches periodically at a user-defined rate which could be slower than the process update date. Rationale: Although records are captured at the end of every process update step, transmitting them to the Monitor at the same rate may not be efficient for high process update frequencies.
- Monitor
  - _Shall_ provide mechanism to start/stop monitoring
  - _Shall_ provide mechanism to select Watchables for display in the appropriate format (2D plot, bar graph, analogue meters, digital indicators)
  - _Shall_ provide mechanism to update Controllables online
  - _Shall_ display rolling statistics for Plant control frequency (i.e. average loop update period and its variance)
  - _Shall_ display cumulative number of dropped records since start of monitoring 
  - _Shall_ provide mechanism to capture data into persistent storage
  - _Shall_ provide mechanism to load and replay previously captured data from persistent storage for post-mortem, on the same GUI.
  - _Should_ provide mechanism to display event log messages from the embedded process
- Plant
  - _Shall_ provide mechanism to set control frequency of the process update loop
  - _Shall_ capture a Record at the end of every process update step (step: single execution of the process update loop)
  - _Shall_ update any changes to Controllables (initiated from the Monitor) at the beginning of a process update step
    - Clarification: Control parameters shall not be modified during the execution of process update step. The sequence is: update control parameter -> run process update step -> capture Record
  - _Shall_ provide a mechanism to turn off all remote monitoring and telemetry, such that the program executes as a normal free-standing application. 
- General
  - _Should_ be POSIX (not just Linux) compatible

## APIs

_TODO_: Update this to a barebones user guide on the API

### Controller

```c++
// Setup variables to watch or control remotely (non-RT)
Controller::pin(const std::string& name, std::span<const T> var, Role role);


Controller::snap(); // Grab a snapshot (RT)
Controller::sync(); // Apply all queued control updates (RT)

// push all snapshots to sink (non-RT)
Controller::flush(); 

// queue a control update (non-RT)
Controller::qset(const std::string& name, std::span<const T> value); 
```

### Monitor

```c++
// subscribe to mulitple signals simultaneously
using Data = std::span<std::byte>;
using Callback = std::function<void(std::span<Signal>, std::span<Data>)>;
Monitor::watch(std::span<std::string> names, Callback cb) -> Error>;

// Set a scalar signal 
template <typename T>
Monitor::set(const std::string& name, const T& value);

// Set a sequence signal
template <typename T>
Monitor::set(const std::string& name, std::span<const T> value) -> Error;

// (optional) Push data to controller
void Monitor::sync();
```

## Similar Projects

- [DataTamer](https://github.com/PickNikRobotics/data_tamer) 
