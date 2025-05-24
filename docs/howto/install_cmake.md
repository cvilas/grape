# How to install CMake

- Use the package manager if the desired version is available there

  ```bash
  wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
  echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
  sudo apt update
  sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
  sudo apt install kitware-archive-keyring
  sudo apt install cmake
  ```

- Build from source for a custom/bleeding-edge version

  ```bash
  sudo apt install libssl-dev

  # Choose verson. Note: Bleeding edge may break third party dependencies (CMake often deprecates
  # features in older versions). Choose wisely
  CMAKE_VERSION=4.0.2
  
  # Download and extract
  wget -SL https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION.tar.gz
  tar -zxvf cmake-$CMAKE_VERSION.tar.gz
  cd cmake-$CMAKE_VERSION
  
  # Build (See README.rst)
  ./bootstrap && make && sudo make install

  # You might have to add the following to your shell configuration (~/.bashrc or ~/.zshrc)
  export PATH=/usr/local/bin:${PATH}
  export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:${LD_LIBRARY_PATH}
  ```
- Install linter/formatter
  
  ```bash
  pipx install cmakelang
  ```