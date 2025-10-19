# README: Teleop

Teleop enables a remote operator to override the robot on-board autonomy stack and take manual 
control of its _locomotion_ functions. 

## Operation Workflow

- Operator gets within line-of-sight (LoS) of the robot and stays within LoS for duration of teleop
- Operator starts teleop console and specifies robot by name. 
- Console starts and presents a terminal UI interface
- Operator requests control of robot. 
- Operator listens for response from locomotion stack confirming or denying control authority
- Operator selects control mode
- Operator generates mode-specific control input and confirms robot motion is as expected
- Operator can view information such as 
  - Network connectivity (latency)
- When done, operator cancels control of robot. 
  - Robot switches back control authority to onboard GNC.

## Design considerations

- Terminal-based client applications for wide deployability, including on the robot itself
- Interacts with [locomotion command arbiter](../arbiter/README.md) to inject control command on the alternate channel

