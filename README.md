# GRAPE- Graphical Real-time Application Prototyping Environment

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

## What is Grape?

GRAPE provides facilities to develop large distributed and embedded control applications, with hooks for instrumentation and remote monitoring. By large, I mean hundreds of nodes on a network of compute units running Linux (X86_64 and Aarch64).

It's also a hobby project that demonstrates [principles](https://github.com/cvilas/guidance) that I follow in software development, and serves as a playground to evaluate [modern C++ techniques](./docs/03_modern_cpp.md) in writing simple, expressive, and performant code for industrial applications. Features of this repo that I consider interesting:

- A [modular build system](./gbs/README.md)
- A performant [logging library](./modules/common/log/README.md)
- A powerful [configuration and scripting system](./modules/common/script/README.md)
- Facilities for low latency [realtime systems](./modules/common/realtime/README.md)
- A stubbornly strict [command line parser](./modules/common/conio/include/grape/conio/program_options.h)
- A scalable [message-passing backend](./modules/common/ipc/README.md)
- Minimal external dependencies

## Getting Started

- [Installation](docs/01_install.md)
- [Roadmap](docs/02_roadmap.md)
