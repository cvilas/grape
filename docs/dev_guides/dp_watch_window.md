# Design Proposal - Supervisor Watch Window

## Purpose
Watch window presents visual information in a way that enables the operator to quickly gather current state and *average* temporal characteristics of signals at a glance. The key design feature is low cognitive load for quick contextual information retrieval. Think analogue speedometer in your car which displays the average trend for your vehicle speed, rather than a 2D trace plot of speed vs time which displays a precise time evolution of the signal but more difficult to read.

## Design principles

- Prefer analogue over digital displays. An analogue display offers both 'state' and 'rate of change of state' information at a glance or over peripheral vision. By contrast, a digital readout demands directed attention, and that information does not typically stay in memory - it's harder to recall a numerical value compared to position of a needle on a dial.

- Allow user to freely *drag-and-drop* rearrange the layout of instruments 

- Automatically save layout on exit. Restore saved layout on restart.

- Follow industry standard designs for standard instrument types.  
