# README: ipc2

## Brief

Intra-process, inter-process and inter-host communication

## Design considerations

- Minimal API that supports following messaging patterns:
  - pub-sub
  - query (remote procedure/service call)
- Automatic discovery and connectivity between matched endpoints
- Configurable for low latency
- Supports high throughput (eg: HD video streams)
- Reliable: Messages get through and in the right order
- Supports non-blocking write operations
- Datatype agnostic: Just provides mechanism to transport raw bytes
- Performant: Minimises data copies
- Scalable to 1000s of endpoints on WiFi without hidden costs (meta data traffic)
- Provides mechanisms to track liveliness of endpoints
- Provides mechanisms to isolate different systems on the same host or network
- Supports interoperability across C++ and Python.

## Zenoh

[Zenoh](https://zenoh.io/docs/overview/what-is-zenoh/) was chosen as the initial candidate for IPC 
after considering a [few options](../../common/ipc/docs/ipc_options.md). 
See [examples](./examples/README.md) for usage patterns and capabilities.

- To evaluate Zenoh, install Rust toolchain

  ```bash
  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

  . "$HOME/.cargo/env"

  # install toolchains for X86_64 and Aarch64
  rustup target add x86_64-unknown-linux-gnu aarch64-unknown-linux-gnu
  rustup toolchain add stable-x86_64-unknown-linux-gnu stable-aarch64-unknown-linux-gnu
  sudo apt install libssl-dev # required for sccache
  cargo install sccache --locked
  ```

- To cache Rust builds, add following lines to `$HOME/.cargo/config.toml`

  ```toml
  [build]
  rustc-wrapper="sccache"
  ```

### Copyright

Use of Zenoh within this project satisfies the terms and conditions of Apache License version 2.0 under which it is distributed.
