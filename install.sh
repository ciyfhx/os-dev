#!/bin/bash

# install dependencies
sudo apt update
sudo apt install build-essential -y
sudo apt install bison -y
sudo apt install flex -y
sudo apt install libgmp3-dev -y
sudo apt install libmpc-dev -y
sudo apt install libmpfr-dev -y
sudo apt install texinfo -y

export PREFIX="/usr/local/x86_64elfgcc" # install location
export TARGET=x86_64-elf # targeted platform
export PATH="$PREFIX/bin:$PATH" # binary location

# compile binutils

TMP="/tmp/os_compile/"
BINUTILS_URL="http://ftp.gnu.org/gnu/binutils/"
BINUTILS_VERSION="binutils-2.42"

mkdir -p $TMP && cd $TMP
echo "Download source from $BINUTILS_URL$BINUTILS_VERSION.tar.xz"
curl -O "$BINUTILS_URL$BINUTILS_VERSION.tar.xz"
tar xf "$BINUTILS_VERSION.tar.xz"
mkdir binutils-build && cd binutils-build
../$BINUTILS_VERSION/configure --target=$TARGET --enable-interwork --enable-multilib --disable-nls --disable-werror --prefix=$PREFIX 2>&1 | tee configure.log
sudo make all install 2>&1 | tee make.log

# compile gcc

GCC_URL="https://ftp.gnu.org/gnu/gcc/"
GCC_VERSION="gcc-14.1.0"

cd $TMP
echo "Download source from $GCC_URL/$GCC_VERSION/$GCC_VERSION.tar.gz"
curl -O "$GCC_URL/$GCC_VERSION/$GCC_VERSION.tar.gz"
tar xf "$GCC_VERSION.tar.gz"
mkdir gcc-build && cd gcc-build
echo Configure: . . . . . . .
../$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-language=c,c++ --without-headers --disable-hosted-libstdcxx
echo MAKE ALL-GCC:
sudo make all-gcc
echo MAKE ALL-TARGET-LIBGCC:
sudo make all-target-libgcc
echo MAKE INSTALL-GCC:
sudo make install-gcc
echo MAKE INSTALL-TARGET-LIBGCC:
sudo make install-target-libgcc

echo MAKE ALL-TARGET-LIBSTDC++:
sudo make all-target-libstdc++-v3 
echo MAKE INSTALL-TARGET-LIBSTDC++:
sudo make install-target-libstdc++-v3

echo Location of installed gcc:

# update path
ls "$PREFIX/bin"
export PATH="$PATH:$PREFIX/bin"