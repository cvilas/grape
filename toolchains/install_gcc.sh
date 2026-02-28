#!/bin/bash

# Install GCC toolchain from Ubuntu Toolchain PPA
# Usage: ./install_gcc.sh

set -e

GCC_VERSION=15

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install -y g++-$GCC_VERSION gcc-$GCC_VERSION gfortran-$GCC_VERSION

PRIORITY=$((${GCC_VERSION%%.*} * 10))
sudo update-alternatives --remove-all gcc || true
sudo update-alternatives --remove-all gfortran || true
sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/bin/gfortran-$GCC_VERSION $PRIORITY
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION $PRIORITY \
  --slave /usr/bin/g++ g++ /usr/bin/g++-$GCC_VERSION \
  --slave /usr/bin/gcov gcov /usr/bin/gcov-$GCC_VERSION
