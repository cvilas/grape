#!/bin/bash

set -e

# Install baseline system utilities and development tools
sudo apt-get update
sudo apt-get install -y \
  software-properties-common build-essential pkg-config gpg wget ca-certificates \
  git-lfs curl ninja-build ccache doxygen graphviz python-is-python3 python3-pip python3-build \
  avahi-daemon avahi-utils iproute2 iputils-ping net-tools iftop htop nvtop patch evtest

# Install support libraries for 3D graphics and GUIs
sudo apt-get install -y \
  libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxkbcommon-dev libwayland-dev wayland-protocols
