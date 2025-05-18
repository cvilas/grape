# Installation

## Supported platforms

OS               |  Architecture   | Compiler
-----------------|-----------------|----------------
Ubuntu 24.04 LTS | Aarch64, X86_64 | clang-21 gcc-14

## Setup build environment

- Install OS utilities 

  ```bash
  sudo apt install software-properties-common build-essential pkg-config gpg wget ca-certificates \
  git-lfs curl ninja-build ccache doxygen graphviz linux-generic python3-full python3-dev \
  python-is-python3 pybind11-dev python3-wheel python3-setuptools python3-build pipx avahi-daemon \
  avahi-utils iproute2 iputils-ping net-tools iftop htop nvtop patch
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
    export CLANG_VERSION=21 # Replace with latest version number
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
    sudo ln -s /usr/lib/llvm-$CLANG_VERSION /usr/lib/llvm-latest
    ```

  - GCC

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

- (Desktop only) For graphical applications, install graphics and windowing libraries 

  ```bash
  sudo apt install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  libxkbcommon-dev libwayland-dev wayland-protocols
  ```

## Configure and build

Using Clang toolchain:

```bash
git clone git@github.com:cvilas/grape
cmake --preset clang
cmake --build build/clang --target all examples check
```

Using GCC:

```bash
git clone git@github.com:cvilas/grape
cmake --preset native
cmake --build build/native --target all examples check
```
