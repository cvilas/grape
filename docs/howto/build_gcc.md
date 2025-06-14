# How to build GCC from source

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
../gcc-$GCC_VERSION/configure --enable-languages=c,c++,fortran --disable-multilib --prefix=$HOME/programs/gcc-$GCC_VERSION
make 
sudo make install

# Configure platform to prefer this new version of GCC
PRIORITY=$((${GCC_VERSION%%.*} * 10))
sudo update-alternatives --remove-all gcc 
sudo update-alternatives --remove-all gfortran
sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/local/bin/gfortran $PRIORITY
sudo update-alternatives --install /usr/bin/gcc gcc /usr/local/bin/gcc $PRIORITY \
--slave /usr/bin/g++ g++ /usr/local/bin/g++ \
--slave /usr/bin/gcov gcov /usr/local/bin/gcov

# You might have to add the following to your shell configuration (~/.bashrc or ~/.zshrc)
export GCC_VERSION=15.1.0
export PATH=${HOME}/programs/gcc-${GCC_VERSION}/bin:/usr/local/bin:${PATH}
export LD_LIBRARY_PATH=${HOME}/programs/gcc-${GCC_VERSION}/lib64/:/usr/local/lib:/usr/local/lib64:${LD_LIBRARY_PATH}
```
