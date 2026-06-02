# Installing toolchains on Pi OS (Debian Trixie)

## Setup additional repo and update the system

```bash
# Add backports
echo 'deb http://deb.debian.org/debian trixie-backports main' | sudo tee /etc/apt/sources.list.d/trixie-backports.list
sudo apt update && sudo apt full-upgrade -y -t trixie-backports 

# Add sid/unstable (warning: see next step)
echo 'deb http://deb.debian.org/debian sid main' | sudo tee /etc/apt/sources.list.d/sid.list

# Pin trixie-backports and trixie higher than sid for package updates
# Warning: Installing packages from `sid` over time _will_ destabilise the system. Therefore,
# set it to lowest priority and install specific packages from it only intentionally:
# `apt policy <package>` tells us versions of the _package_ available across repos. 

sudo tee /etc/apt/preferences.d/99-trixie-sid >/dev/null <<'EOF'
Package: *
Pin: release n=trixie-backports
Pin-Priority: 900

Package: *
Pin: release n=trixie
Pin-Priority: 800

Package: *
Pin: release n=sid
Pin-Priority: 100
EOF

sudo apt update
sudo apt full-upgrade -y
```

## Install 'base' packages

See [toolchains/install_base](../../../toolchains/install_base.sh)

## Install cmake 4.x

```bash
sudo apt install -t trixie-backports cmake
```

## Install LLVM toolchain

```bash
LLVM_VERSION=22
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh $LLVM_VERSION

sudo apt-get install -y \
  clang-$LLVM_VERSION llvm-$LLVM_VERSION-dev libc++-$LLVM_VERSION-dev libc++abi-$LLVM_VERSION-dev \
  libomp-$LLVM_VERSION-dev libunwind-$LLVM_VERSION-dev lld-$LLVM_VERSION
sudo apt-get autoremove -y

# Set this version of llvm as the default
PRIORITY=$((${LLVM_VERSION%%.*} * 10))
sudo update-alternatives --remove-all clang || true
sudo update-alternatives --remove-all clang++ || true
sudo update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-$LLVM_VERSION/bin/clang $PRIORITY
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-$LLVM_VERSION/bin/clang++ $PRIORITY
sudo update-alternatives --install /usr/bin/lld lld /usr/lib/llvm-$LLVM_VERSION/bin/lld $PRIORITY
sudo update-alternatives --install /usr/bin/ld.lld ld.lld /usr/lib/llvm-$LLVM_VERSION/bin/ld.lld $PRIORITY
```

## Install gcc toolchain (from sid/unstable)

```bash
GCC_VERSION=16

# Simulate first: do this before installing anything. This shows exactly what APT would pull in
sudo apt -s install -t sid g++-$GCC_VERSION gcc-$GCC_VERSION gfortran-$GCC_VERSION binutils \
  libgcc-$GCC_VERSION-dev libstdc++-$GCC_VERSION-dev

# If the simulation looks acceptable, install
sudo apt-get install -y -t sid g++-$GCC_VERSION gcc-$GCC_VERSION gfortran-$GCC_VERSION binutils \
  libgcc-$GCC_VERSION-dev libstdc++-$GCC_VERSION-dev

PRIORITY=$((${GCC_VERSION%%.*} * 10))
sudo update-alternatives --remove-all gcc || true
sudo update-alternatives --remove-all gfortran || true
sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/bin/gfortran-$GCC_VERSION $PRIORITY
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION $PRIORITY \
  --slave /usr/bin/g++ g++ /usr/bin/g++-$GCC_VERSION \
  --slave /usr/bin/gcov gcov /usr/bin/gcov-$GCC_VERSION
```

