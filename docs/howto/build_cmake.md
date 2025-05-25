# How to build CMake from source

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