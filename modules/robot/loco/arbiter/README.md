# README: Locomotion arbiter

## Brief

Locomotion arbiter resolves control authority over robot locomotion stack

![Locomotion Command Arbitration](./docs/loco_command_arbiter.drawio.svg)

## Detailed description

- The arbiter arbitrates control authority between two sources of locomotion commands
  - `Primary`, via direct function call (e.g. from autonomous navigation stack)
  - `Alternate`, via IPC (e.g. from remote manual teleoperation client)
- The arbiter expects a continuous periodic stream of locomotion commands from its sources. Typically this should be greater than 10 Hz.
- Nominally the command arbiter acts on the _primary_ source for locomotion commands.
- If locomotion command is detected on _alternate_ channel, the arbiter transfers control authority to that source.
- If locomotion command stops on the _alternate_ channel, the arbiter transfers control back to the primary source after a timeout period.
- At most one _alternate_ source can be active at any time. If multiple alternate sources send command streams, the arbiter ignores them from all except one.

See further documentation inline
