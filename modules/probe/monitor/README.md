# README: monitor

## Brief

monitor provides .. <one line description>

## Detailed description

Provide more details on the functionality, API usage, etc

### Third party libs

- [GLFW3](https://www.glfw.org/): Hardware accelerated graphics
  - To get OpenGL3 working on qemu VM in MacOS, turn off hardware acceleration by using one of the non-gl display hardware options.
  - `glxinfo -B` on the terminal lists OpenGL versions available on the host
- [imgui](https://github.com/ocornut/imgui): Immediate mode 2D graphics
- [implot](https://github.com/epezent/implot): Pretty plotting for imgui

## TODO

- Implement data transport server (Controller) and client (Monitor)
  - Implement PlotJuggler realtime streaming plugin using zenoh multicast for transport
- Incorporate changes to PoC.
  - :done: Make it easy to identify timestamp field with `Signal::Role::Timestamp`  
  - :done: Throw error if exactly one instance of `Signal::Role::Timestamp` is not defined
  - Reorganise data as struct of arrays instead of array of struct
  - Use latest timestamp to set history of a plot (X axis limits must come from time in data history rather than monitor clock)
  - Refactor monitor to support single signals as well as frames of multiple signals. This involves redesigning monitor to essentially take single (time, signal) pairs as inputs and having a separate function to demultiplex composite frames to individual (time, signal). This should also help with the implementation of mcap writer (but not csv writer)
    - Refactor `ScrollBuffer` to hold a single signal.
    - Demux composite frame into multiple scroll buffers
    - Define sink (`Monitor::recv`) for a single signal, in addition to frame of signals
  - Optimise scroll buffer to only hold watched signals. Use the fact that they are sorted (or not) to find the span of watched variables
  - Optimise controls buffer by only reading control variables from (signal,frame). Use the fact that they are sorted (or not) to find the span of control variables
  - Set latest controls data in controls buffer. Show current value next to control input
  - Implement signals list window
  - Implement signal show/hide button
  - Implement 'freeze' for each graph
  - Redesign to accommodate monitors and data writers (mcap) simultaneously
- Choose archival data format: CSV, [MCAP](https://github.com/foxglove/mcap/tree/main/cpp), [PlotJuggler](https://github.com/facontidavide/PlotJuggler)
- Implement data archive reader and writer
- Implement Monitor application
  - Server connection window
  - Controllables group windows
  - Data recording controls window
  - Data playback controls window
  - Save session configuration to file
  - Restore session configuration from file
- Update this README
