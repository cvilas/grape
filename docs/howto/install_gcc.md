# How to install GCC toolchain

- Use the package manager if the desired version is available there

  ```bash
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  GCC_VERSION=15
  sudo apt update
  sudo apt install g++-$GCC_VERSION gcc-$GCC_VERSION gfortran-$GCC_VERSION
  
  PRIORITY=${GCC_VERSION%%.*}
  sudo update-alternatives --remove-all gcc 
  sudo update-alternatives --remove-all gfortran
  sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/bin/gfortran-$GCC_VERSION $PRIORITY
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION $PRIORITY \
  --slave /usr/bin/g++ g++ /usr/bin/g++-$GCC_VERSION \
  --slave /usr/bin/gcov gcov /usr/bin/gcov-$GCC_VERSION
  
  # (X86_64 only): install Aarch64 cross compilers 
  sudo apt install g++-$GCC_VERSION-aarch64-linux-gnu gcc-$GCC_VERSION-aarch64-linux-gnu \
  gfortran-$GCC_VERSION-aarch64-linux-gnu binfmt-support qemu-user-static qemu-system-arm -y
  
  sudo update-alternatives --remove-all aarch64-linux-gnu-gcc aarch64-linux-gnu-gfortran
  sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gfortran aarch64-linux-gnu-gfortran /usr/bin/aarch64-linux-gnu-gfortran-$GCC_VERSION $PRIORITY
  sudo update-alternatives --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-$GCC_VERSION $PRIORITY \
  --slave /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-$GCC_VERSION \
  --slave /usr/bin/aarch64-linux-gnu-gcov aarch64-linux-gnu-gcov /usr/bin/aarch64-linux-gnu-gcov-$GCC_VERSION
  ```

- Build from source for a custom/bleeding-edge version

  ```bash
  GCC_VERSION=15.1.0 
  
  # Download sources
  wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz
  tar xf gcc-$GCC_VERSION.tar.xz
  cd gcc-$GCC_VERSION
  ./contrib/download_prerequisites
  
  # Configure, build (many hours for Pi5/4GB), install
  cd ..
  mkdir gcc-build && cd gcc-build
  ../gcc-$GCC_VERSION/configure --enable-languages=c,c++,fortran --disable-multilib
  make 
  sudo make install
  
  # Configure platform to prefer this new version of GCC
  PRIORITY=${GCC_VERSION%%.*}
  sudo update-alternatives --remove-all gcc 
  sudo update-alternatives --remove-all gfortran
  sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/local/bin/gfortran $PRIORITY
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/local/bin/gcc $PRIORITY \
  --slave /usr/bin/g++ g++ /usr/local/bin/g++ \
  --slave /usr/bin/gcov gcov /usr/local/bin/gcov
  
  # You might have to add the following to your shell configuration (~/.bashrc or ~/.zshrc)
  export PATH=/usr/local/bin:${PATH}
  export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:${LD_LIBRARY_PATH}
  ```
