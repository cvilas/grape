#!/bin/bash

set -e

# Install baseline system utilities and development tools
sudo apt-get update
sudo apt-get install -y \
  software-properties-common build-essential pkg-config gpg wget ca-certificates \
  git-lfs curl ninja-build ccache doxygen graphviz python-is-python3 python3-pip \
  python3-build python3-venv avahi-daemon avahi-utils iproute2 iputils-ping \
  net-tools iw iftop htop nvtop patch evtest

# Install support libraries for 3D graphics and GUIs
sudo apt-get install -y libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev \
libxi-dev libxss-dev libxtst-dev libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev \
libgles2-mesa-dev libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev libthai-dev \
libpipewire-0.3-dev libwayland-dev libdecor-0-dev liburing-dev

