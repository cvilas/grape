#!/bin/bash

# Installs the LLVM toolchain on a Debian-based system.
# Usage: ./install_llvm.sh
# This script is intended to be run with root privileges.

set -e

LLVM_VERSION=22

sudo apt-get install -y wget gnupg lsb-release software-properties-common

wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc

CODENAME=$(lsb_release -cs)
sudo add-apt-repository "deb http://apt.llvm.org/${CODENAME}/ llvm-toolchain-${CODENAME}-${LLVM_VERSION} main"

sudo apt-get update

sudo apt-get install -y \
  clang-$LLVM_VERSION clang-tidy-$LLVM_VERSION clang-format-$LLVM_VERSION \
  llvm-$LLVM_VERSION-dev libc++-$LLVM_VERSION-dev libc++abi-$LLVM_VERSION-dev \
  libomp-$LLVM_VERSION-dev libunwind-$LLVM_VERSION-dev lld-$LLVM_VERSION
sudo apt-get autoremove -y

# Set this version as the default
PRIORITY=$((${LLVM_VERSION%%.*} * 10))
sudo update-alternatives --remove-all clang || true
sudo update-alternatives --remove-all clang++ || true
sudo update-alternatives --remove-all clang-tidy || true
sudo update-alternatives --remove-all clang-format || true
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$LLVM_VERSION $PRIORITY
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-$LLVM_VERSION $PRIORITY
sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-$LLVM_VERSION $PRIORITY
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-$LLVM_VERSION $PRIORITY
sudo update-alternatives --install /usr/bin/lld lld /usr/bin/lld-$LLVM_VERSION $PRIORITY
sudo update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-$LLVM_VERSION $PRIORITY
