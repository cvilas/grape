# How to build CMake from source

```bash
sudo apt install libssl-dev

export CMAKE_VERSION=3.31.7 # latest as of May 2025. Replace with your desired version
wget https://github.com/Kitware/CMake/archive/refs/tags/v$CMAKE_VERSION.tar.gz -O cmake-$CMAKE_VERSION.tar.gz
tar -zxvf cmake-$CMAKE_VERSION.tar.gz
cd CMake-$CMAKE_VERSION
./bootstrap && gmake -j4 && sudo make install
 ```