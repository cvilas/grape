# GRAPE- Graphical Real-time Application Prototyping Environment

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

## What is Grape?

- A monorepo that implements building blocks for highly distributed robotic systems. Specifically, GRAPE can be used to build systems consisting of hundreds of process nodes running on hundreds of compute units (Linux X86_64/Aarch64) across a LAN.
- A playground to evaluate modern C++ techniques in writing simple, expressive, and performant code for industrial applications
- A hobby project that demonstrates the application of systems engineering [guidelines](https://github.com/cvilas/guidance) that I developed over decades in the industry as a technical and people leader in industrial robotics. ([Author's profile](https://cvilas.github.io/))

Selected features:

- A [modular build system](./gbs/README.md)
- A fast [logging library](./modules/common/log/README.md)
- A fast [serialisation library](./modules/common/serdes/README.md)
- A powerful [configuration and scripting system](./modules/common/script/README.md)
- Facilities for low latency [realtime systems](./modules/common/realtime/README.md)
- A stubbornly strict [command line parser](./modules/common/conio/README.md)
- A high performance [inter-process communication library](./modules/common/ipc/README.md)
- An [application development framework](./docs/howto/write_applications.md)
- Support for the latest C++ standard, compilers and tooling
- Focus on [Raspberry Pi](https://www.raspberrypi.com/) as the target embedded hardware platform

## Getting Started

- [Installation](docs/01_install.md)
- [Roadmap](docs/02_roadmap.md)