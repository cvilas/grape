# Installation

## Supported platforms

OS               | Architecture  | Compiler
-----------------|---------------|----------------
Ubuntu 22.04 LTS | Arm64, X86_64 | GCC13, Clang18

## Setup build environment

- Install the basic tools
  ```bash
  sudo apt install pkg-config gpg wget ca-certificates git-lfs curl ccache ninja-build \
  doxygen graphviz linux-generic python3-dev iproute2 python-is-python3 net-tools iftop htop
  ```
- Install latest cmake
  ```bash
  wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
  echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
  sudo apt update
  sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
  sudo apt install kitware-archive-keyring
  sudo apt install cmake
  ```
- Install latest compilers
  - Clang
    ```bash
    wget https://apt.llvm.org/llvm.sh
    export CLANG_VERSION=18 # Replace with latest version number
    chmod +x llvm.sh
    sudo ./llvm.sh $CLANG_VERSION
    sudo apt install clang-$CLANG_VERSION clang-tidy-$CLANG_VERSION clang-format-$CLANG_VERSION \
    llvm-$CLANG_VERSION-dev libc++-$CLANG_VERSION-dev libomp-$CLANG_VERSION-dev libc++abi-$CLANG_VERSION-dev \
    libunwind-$CLANG_VERSION-dev
    sudo update-alternatives --remove-all clang 
    sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$CLANG_VERSION $CLANG_VERSION \
    --slave /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VERSION \
    --slave /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-$CLANG_VERSION \
    --slave /usr/bin/clang-format clang-format /usr/bin/clang-format-$CLANG_VERSION 
    ```
  - GCC
    ```bash
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    export GCC_VERSION=13 # Replace with latest version number
    sudo apt update
    sudo apt install g++-$GCC_VERSION gcc-$GCC_VERSION gfortran-$GCC_VERSION
    
    sudo update-alternatives --remove-all gcc gfortran
    sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/bin/gfortran-$GCC_VERSION $GCC_VERSION
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION $GCC_VERSION \
    --slave /usr/bin/g++ g++ /usr/bin/g++-$GCC_VERSION \
    --slave /usr/bin/gcov gcov /usr/bin/gcov-$GCC_VERSION
    
    # (X86_64 only): install Arm64 cross compilers 
    sudo apt install g++-$GCC_VERSION-aarch64-linux-gnu gcc-$GCC_VERSION-aarch64-linux-gnu \
    gfortran-$GCC_VERSION-aarch64-linux-gnu binfmt-support qemu-user-static qemu-system-arm qemu -y

  
    sudo update-alternatives --remove-all aarch64-linux-gnu-gcc aarch64-linux-gnu-gfortran
    sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gfortran aarch64-linux-gnu-gfortran /usr/bin/aarch64-linux-gnu-gfortran-$GCC_VERSION $GCC_VERSION
    sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-$GCC_VERSION $GCC_VERSION \
    --slave /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-$GCC_VERSION \
    --slave /usr/bin/aarch64-linux-gnu-gcov aarch64-linux-gnu-gcov /usr/bin/aarch64-linux-gnu-gcov-$GCC_VERSION
    ```
  - Rust
    ```bash
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
    ```
    
## Configure and build

Note (Dec 2023) Use the clang toolchain to build the project. GCC-13 does not support all features we use.

```bash
git clone git@github.com:cvilas/grape
mkdir -p grape/build
cd grape/build
cmake .. -DBUILD_MODULES=all -DCMAKE_TOOLDCHAIN_FILE=$PWD/../toolchains/toolchain_clang.cmake -GNinja
ninja
```
