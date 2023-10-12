# GRAPE- Graphical Real-time Application Prototyping Environment

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

## What is Grape?

GRAPE is a framework to develop large distributed and embedded control applications, with hooks for instrumentation and remote monitoring. By large, I mean hundreds of nodes on a network of compute units running Linux (X86_64 and Aarch64).

It's also a hobby project; an experimental playground to learn/evaluate/apply modern C++ techniques in writing simple, expressive, and performant code for industrial applications. Some features of this repo that I consider interesting

- A [modular build system](./gbs/README.md)
- A simple [command line parser](./modules/common/utils/src/command_line_args.cpp)
- A simple [logging library](./modules/common/log/README.md)
- A powerful [configuration and scripting system](./modules/common/script/README.md) 
- Minimal external dependencies
- Dependence on the bleeding edge for development tools:
  - Latest C++ language version
  - Latest release of compilers (Clang and GCC)
  - Latest CMake

## Getting Started

- [Installation](docs/01_install.md)
- [Roadmap](docs/02_roadmap.md)
