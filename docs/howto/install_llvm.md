# How to install LLVM toolchain

```bash
wget https://apt.llvm.org/llvm.sh
wget -O- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
CLANG_VERSION=21 
chmod +x llvm.sh
sudo ./llvm.sh $CLANG_VERSION
sudo apt install clang-$CLANG_VERSION clang-tidy-$CLANG_VERSION clang-format-$CLANG_VERSION \
llvm-$CLANG_VERSION-dev libc++-$CLANG_VERSION-dev libomp-$CLANG_VERSION-dev libc++abi-$CLANG_VERSION-dev \
libunwind-$CLANG_VERSION-dev lld-$CLANG_VERSION

PRIORITY=${CLANG_VERSION%%.*}
sudo update-alternatives --remove-all clang 
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-$CLANG_VERSION $PRIORITY \
--slave /usr/bin/clang++ clang++ /usr/bin/clang++-$CLANG_VERSION \
--slave /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-$CLANG_VERSION \
--slave /usr/bin/clang-format clang-format /usr/bin/clang-format-$CLANG_VERSION 
sudo ln -s /usr/lib/llvm-$CLANG_VERSION /usr/lib/llvm-latest
```