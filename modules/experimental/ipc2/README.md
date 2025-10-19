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
- Add the following to `external/third_party_versions.cmake`

```
set(ZENOH_VERSION_REQUIRED        1.1.0)
set(ZENOHC_VERSION_REQUIRED       ${ZENOH_VERSION_REQUIRED})
set(ZENOHCXX_VERSION_REQUIRED     ${ZENOH_VERSION_REQUIRED})
```

- Add the following section to `external/CMakeLists.txt`

```
# -------------------------------------------------------------------------------------------------
# zenohcxx
if(zenohcxx IN_LIST EXTERNAL_PROJECTS_LIST)
  # install zenoh-c first
  find_package(zenohc ${ZENOHC_VERSION_REQUIRED} QUIET)
  if(zenohc_FOUND)
    message(STATUS "zenohc: Using version ${zenohc_VERSION} from ${zenohc_DIR}")
    add_dummy_target(zenohc)
  else()
    message(STATUS "zenohc: Building ${ZENOHC_VERSION_REQUIRED} from source")
    set(ZENOHC_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DZENOHC_CARGO_CHANNEL=+stable
        -DZENOHC_CUSTOM_TARGET=${RUSTC_TRIPLE}
        -DZENOHC_BUILD_WITH_UNSTABLE_API=ON
        -DZENOHC_BUILD_WITH_SHARED_MEMORY=ON # shm requires unstable API for now (Dec 2024)
        -DCMAKE_VERBOSE_MAKEFILE=ON)
    ExternalProject_Add(
      zenohc
      GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-c.git"
      GIT_TAG ${ZENOHC_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${ZENOHC_CMAKE_ARGS})
  endif()

  find_package(zenohcxx ${ZENOHCXX_VERSION_REQUIRED} QUIET)
  if(zenohcxx_FOUND)
    message(STATUS "zenohcxx: Using version ${zenohcxx_VERSION} from ${zenohcxx_DIR}")
    add_dummy_target(zenohcxx)
  else()
    message(STATUS "zenohcxx: Building ${ZENOHCXX_VERSION_REQUIRED} from source")
    set(ZENOHCXX_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DZENOHCXX_ZENOHC=ON 
        -DZENOHCXX_ZENOHPICO=OFF
        -DZENOHCXX_EXAMPLES_PROTOBUF=OFF)
    ExternalProject_Add(
      zenohcxx
      GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-cpp.git"
      GIT_TAG ${ZENOHCXX_VERSION_REQUIRED}
      GIT_SHALLOW true
      DEPENDS zenohc
      CMAKE_ARGS ${ZENOHCXX_CMAKE_ARGS})
  endif()
endif()
```

### Copyright

Use of Zenoh within this project satisfies the terms and conditions of Apache License version 2.0 under which it is distributed.

## TODO

- Implement queryable/query API
- Implement zero-copy read and write
- Implement Reliable/BestEffort QoS
- Implement History QoS
- Define topics for matched example programs in a single place
- Implement PutOptions and subscriber Sample fields
  - Support attachments
  - Resolve how we can combine congestion control, priority and reliability settings in a coherent way to offer fewer choices at the user API layer?
    - See [discord](https://discord.com/channels/914168414178779197/940584045287460885/1311629493445853206)
  - Consider supporting sample kind (put/delete)
- Understand the point of on_drop callback in subscriber and support it if necessary
- Documentation cleanup: examples
- Understand hybrid logical clocks
- Support hybrid logical clocks implementation
- Fix zenoh examples: pull, shm pub/sub
- New zenoh examples: Router interceptors (downsampling), authentication, access control, serdes (ZBytes)
- PoC IPC experiments
  - Case 1: pub-peer on PC1, sub-peer on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 2: pub-peer + router on PC1, sub-peer + router on PC2, router on PC3, multicast scouting off. Confirm data transfer from PC1 to PC2, no data transfer through PC3.
  - Case 3: Extend case2 by adding a PC4 with router and sub-client. Confirm sub-client on PC4 receives data from pub-peer on PC1.
