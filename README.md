# GRAPE- Graphical Real-time Application Prototyping Environment

[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)

Grape consists of two components supplied as independent libraries - grape_plant and grape_supervisor. *grape_plant* 
provides a framework to implement closed loop control, and is potentially deployed on an embedded processor board - the 
*Plant* in control systems terminology. *grape_supervisor* provides mechanisms for remote monitoring, graphing, online 
parameter tuning of Plant control parameters, and event logging; at the very least, the supervisor runs as a separate 
process, more likely it runs on a different host. The supervisor and the plant communicate via message passing.
