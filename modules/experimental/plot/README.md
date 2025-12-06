# plot

Realtime signal plotting

## Design considerations

- Simple API
```C++

template<Plottable T>
void plot(Plotter& plotter, const T& data)
```

## Plottable Concept

The `Plottable` concept defines the requirements for time series data types that can be plotted.

### Requirements

A type `T` satisfies `Plottable` if:

1. **Has a timestamp field**: The type must contain a member named `timestamp` of any `std::chrono::time_point` type (any clock and duration)
2. **Is an aggregate type**: The type must be a struct with public members (aggregate initialization supported)
3. **Has standard layout**: The type must have a predictable memory layout for efficient access
4. **Contains arithmetic fields**: Fields (other than timestamp) should be integral or floating point types

### Example Usage

```cpp
#include "grape/plottable.h"
#include <chrono>

using SteadyNs = std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>;

// Define a plottable data structure
struct SensorData {
	SteadyNs timestamp;
	double temperature;
	int32_t pressure;
	float humidity;
};

// Verify at compile-time
static_assert(grape::Plottable<SensorData>);

// Use with generic plotting functions
template<grape::Plottable T>
void plot(Plotter& plotter, const T& data) {
	// Implementation
}
```

### Supported Timestamp Types

Any `std::chrono::time_point<Clock, Duration>`
- Any clock (e.g., `steady_clock`, `system_clock`)
- Any duration (e.g., `nanoseconds`, `microseconds`, `milliseconds`, `seconds`)

### Supported Data Field Types

- **Integral types**: `int8_t`, `int16_t`, `int32_t`, `int64_t`, `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`
- **Floating point types**: `float`, `double`

See [examples/plottable_example.cpp](examples/plottable_example.cpp) for more examples.