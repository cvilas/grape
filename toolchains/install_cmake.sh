#!/bin/bash

# Installs CMake on Ubuntu/Debian from Kitware's APT repository.
# Usage: ./install_cmake.sh
# Requires sudo privileges.

set -e

wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
  gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | \
  sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

sudo apt update
sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
sudo apt install -y kitware-archive-keyring
sudo apt install -y cmake