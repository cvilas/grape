# Installation

## Supported platforms

OS               |  Architecture   | Compiler
-----------------|-----------------|----------------
Ubuntu 24.04 LTS | Aarch64, X86_64 | clang-20 gcc-14

## Setup build environment

- Install the basic tools

  ```bash
  sudo apt install build-essential pkg-config gpg wget ca-certificates git-lfs curl ninja-build \
  ccache doxygen graphviz linux-generic python3-dev python-is-python3 pipx \
  iproute2 iputils-ping net-tools iftop htop nvtop
  ```

- Install latest cmake and helpers

  ```bash
  wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
  echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
  sudo apt update
  sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
  sudo apt install kitware-archive-keyring
  sudo apt install cmake
  pipx install cmakelang
  ```

- Install latest compilers
  - Clang

    ```bash
    wget https://apt.llvm.org/llvm.sh
    wget -O- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    export CLANG_VERSION=20 # Replace with latest version number
    chmod +x llvm.sh
    sudo ./llvm.sh $CLANG_VERSION
    sudo apt install clang-$CLANG_VERSION clang-tidy-$CLANG_VERSION clang-format-$CLANG_VERSION \
    llvm-$CLANG_VERSION-dev libc++-$CLANG_VERSION-dev libomp-$CLANG_VERSION-dev libc++abi-$CLANG_VERSION-dev \
    libunwind-$CLANG_VERSION-dev lld-$CLANG_VERSION
    sudo update-alternatives --remove-all clang 
    sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$CLANG_VERSION $CLANG_VERSION \
    --slave /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VERSION \
    --slave /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-$CLANG_VERSION \
    --slave /usr/bin/clang-format clang-format /usr/bin/clang-format-$CLANG_VERSION 
    ```

  - GCC (Your mileage may vary!)

    ```bash
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    export GCC_VERSION=14 # Replace with latest version number
    sudo apt update
    sudo apt install g++-$GCC_VERSION gcc-$GCC_VERSION gfortran-$GCC_VERSION
    
    sudo update-alternatives --remove-all gcc 
    sudo update-alternatives --remove-all gfortran
    sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/bin/gfortran-$GCC_VERSION $GCC_VERSION
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION $GCC_VERSION \
    --slave /usr/bin/g++ g++ /usr/bin/g++-$GCC_VERSION \
    --slave /usr/bin/gcov gcov /usr/bin/gcov-$GCC_VERSION
    
    # (X86_64 only): install Aarch64 cross compilers 
    sudo apt install g++-$GCC_VERSION-aarch64-linux-gnu gcc-$GCC_VERSION-aarch64-linux-gnu \
    gfortran-$GCC_VERSION-aarch64-linux-gnu binfmt-support qemu-user-static qemu-system-arm -y

    sudo update-alternatives --remove-all aarch64-linux-gnu-gcc aarch64-linux-gnu-gfortran
    sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gfortran aarch64-linux-gnu-gfortran /usr/bin/aarch64-linux-gnu-gfortran-$GCC_VERSION $GCC_VERSION
    sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-$GCC_VERSION $GCC_VERSION \
    --slave /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-$GCC_VERSION \
    --slave /usr/bin/aarch64-linux-gnu-gcov aarch64-linux-gnu-gcov /usr/bin/aarch64-linux-gnu-gcov-$GCC_VERSION
    ```

  - Rust

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

- (Desktop only) For graphical applications, install GL and windowing libraries 

  ```bash
  sudo apt install libgl-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  libxkbcommon-dev libwayland-dev wayland-protocols
  ```

## Configure and build

Note (Dec 2024): Clang toolchain is recommended to build the project:

```bash
git clone git@github.com:cvilas/grape
mkdir -p grape/build
cd grape/build
cmake .. -DBUILD_MODULES=all -DCMAKE_TOOLCHAIN_FILE=$PWD/../toolchains/toolchain_clang.cmake -GNinja
ninja
```

With gcc, use `Makefile` instead of `ninja` to build libraries and tests. Some examples do not build
because libstdc++ does not support formatted printing over containers

```bash
git clone git@github.com:cvilas/grape
mkdir -p grape/build_gcc
cd grape/build_gcc
cmake .. -DBUILD_MODULES=all
make -j8
```
