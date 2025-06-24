# README: Teleop

Teleop enables a remote operator to override the robot on-board autonomy stack and take manual 
control of it's _locomotion_ functions. 

## Operation Workflow

- Operator gets within line-of-sight (LoS) of the robot and stays within LoS for duration of teleop
- Operator starts teleop console and specifies robot by name. 
- Console starts and presents a terminal UI interface
- Operator 'acquires' control of robot. 
  - Robot responds confirming or denying control authority
- Operator selects control mode
- Operator generates mode-specific control input and confirms robot motion is as expected
- Operator can view information such as 
  - Network connectivity (latency)
- When done, operator 'releases' control of robot. 
  - Robot switches back control authority to onboard GNC.

## Design considerations

- Terminal-based client applications for wide deployability, including on the robot itself
- See server side [README](../service/README.md) for expected behaviour

## TODO

- keyboard teleop console using ftxui
- consider reporting periodic server status reports when teleop is active
  - fixes: When multiple clients exist, not all clients receive feedback
  - Report mean command latency with the report
- server side unit test
- IPC endpoints templated on TopicAttributes concept
