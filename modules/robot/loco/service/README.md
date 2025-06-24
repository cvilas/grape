# README: Locomotion service

## Brief

Locmotion service responds to motion commands from two sources

- Autonomous navigation stack
- Remote teleoperation client

## Behaviour

- Nominally, the service listens to autonomous navigation stack for motion commands. 
- Control authority transfers to remote teleop client when such a client is active. 
- Control automatically transfers back to autonomous navigation stack when teleop client goes inactive
- Atmost one remote teleop client can be active at any time 
- Service expects periodic messages (i.e. stream) from Clients, potentially at tens of messages per second or more
