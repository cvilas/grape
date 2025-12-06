# Modern QMotor Concept

Build a better QMotor, for modern robotics experiments

What makes [QMotor](./crb_qmotor.jpg) awesome as a tool for real-time system development

- Hardware servers at highest priority
- GUI to tune control parameters and view realtime plots
- Programs run in non-privileged mode.
- Program can run without gui
- GUI cannot delay control program as it runs at a lower priority
- Server scripts (configuration)
- Log variables window - multiple parameters can be selected at once to change logging configuration
- Plot window allows multiple logs to select, color, realtime update, mouse over on plot, scroll bars
- Multi window interface
- Global options for display format
- Entire configuration saved to disk
- Runs on PC - no dsp required
- Uses QNX for realtime operation
- Structure of a qmotor program template. Easy to build upon.
- Simple GUI.
- Reliable. 
- Detachable gui. Use gui for tuning, command line for deployment.
- Client-Server architecture
  - Decouples hardware (sensors, actuators) from software (intelligence)
  - Hardware servers allow sharing hardware (timer, I/O board) with multiple clients
- Online control parameter tuning
  - Predefined min, max and default values
  - Log when parameter was modified
  - On the plant side, variable change happens at user determined points in code. Supervisor is informed when a change in parameter goes live on the plant side
- Parameter logging and plotting
  - Realtime plot
  - plotting functionality details...
  - logging functionality details...
  - Watch windows
- Real-time math library 
- Linear algebra and filter function blocks
- Message passing (handleMessages)
- Status messages
- Configuration files
- Error handling - math, timing, IO
