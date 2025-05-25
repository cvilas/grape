#!/bin/bash

set -e

# Install baseline system utilities and development tools
sudo apt-get update
sudo apt-get install -y \
  software-properties-common build-essential pkg-config gpg wget ca-certificates \
  git-lfs curl ninja-build ccache doxygen graphviz python3-full python3-dev \
  python-is-python3 pybind11-dev python3-wheel python3-setuptools python3-build pipx avahi-daemon \
  avahi-utils iproute2 iputils-ping net-tools iftop htop nvtop patch

# Install Python tool for CMake formatting
pipx install cmakelang

# Install support libraries for 3D graphics and GUIs
sudo apt-get install -y \
  libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxkbcommon-dev libwayland-dev wayland-protocols