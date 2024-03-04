# README: monitor

## Brief

monitor provides .. <one line description>

## Detailed description

Provide more details on the functionality, API usage, etc

### Third party libs

- [GLFW3](https://www.glfw.org/): Hardware accelerated graphics
  Note: To get OpenGL3 working on qemu VM in MacOS, turn off hardware acceleration by using one of the non-gl display hardware options. `glxinfo -B` on the terminal lists supported OpenGL versions
- [imgui](https://github.com/ocornut/imgui): Immediate mode 2D graphics
- [implot](https://github.com/epezent/implot): Pretty plotting for imgui

## TODO

- Move 3D backend from SDL3/OpenGL3 to GLFW/OpenGL3?
  - Read [GLFW docs](https://www.glfw.org/docs/latest/)
  - make implot_example work in VM running in MacOS
  - Update install docs: `sudo apt install libgl-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxkbcommon-dev`
- Incorporate changes to PoC.
  - Make it easy to identify timestamp field Options:
    - A flag in the Signal struct?
    - A `PinConfig::setTimestampEntry()` to specify entry for timestamp
    - A `Signal::Role::Timestamp` special enum to specify time  
  - Use latest timestamp to set history of plots
  - Optimise scroll buffer to only hold watched signals. Use the fact that they are sorted (or not) to find the span of watched variables
  - Optimise controls buffer by only reading control variables from (signal,frame). Use the fact that they are sorted (or not) to find the span of control variables
  - Set latest controls data in controls buffer. Show current value next to control input
  - Implement signals list window
  - Implement signal show/hide button
  - Implement 'freeze' for each graph
  - Redesign to accommodate monitors and data writers (mcap) simultaneously
- Choose archival data format (consider ease of reading and writing): CSV, [MCAP](https://github.com/foxglove/mcap/tree/main/cpp), [PlotJuggler](https://github.com/facontidavide/PlotJuggler), custom
- Implement data archive reader and writer
- Design data transport server (Controller) and client (Monitor)
- Implement data transport server and client 
- Review requirements doc and design a solution for each
- Implement Monitor application
  - Server connection window
  - Controllables group windows
  - Data recording controls window
  - Data playback controls window
  - Save session configuration to file
  - Restore session configuration from file 
