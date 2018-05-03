#!/bin/sh
set -e
set -x

# Build and install devkitARM + ctrulib
mkdir -p $DEVKITPRO
cd $DEVKITPRO
wget -N https://raw.githubusercontent.com/devkitPro/installer/master/perl/devkitARMupdate.pl
chmod +x ./devkitARMupdate.pl
./devkitARMupdate.pl

# Get latest ctrulib and overwrite bundled one
git clone https://github.com/smealum/ctrulib.git
cd ctrulib/libctru
make && make install
cd ../../
cp -rf ctrulib/libctru/ ${DEVKITPRO}
rm -rf ctrulib

# Get latest version of Citro3D
git clone --recursive https://github.com/fincs/citro3d.git
cd citro3d
make && make install
cd ../
rm -rf citro3d

# Install latest version of picasso
git clone https://github.com/fincs/picasso.git
cd picasso
./autogen.sh
./configure
make
cp picasso ${DEVKITPRO}/devkitARM/bin/picasso
cd ../
rm -rf picasso

export CC=arm-none-eabi-gcc
export CXX=arm-none-eabi-g++

# Build and install bannertool
mkdir bannertool && cd bannertool
wget https://github.com/Steveice10/bannertool/releases/download/1.1.0/bannertool.zip
unzip bannertool.zip
cp linux-x86_64/bannertool ${DEVKITPRO}/devkitARM/bin/bannertool
cd ..
rm -rf bannertool

# Build and install makerom
git clone https://github.com/profi200/Project_CTR.git
cd Project_CTR/makerom
make clean && make
cp makerom ${DEVKITPRO}/devkitARM/bin/makerom
cd ../..
rm -rf Project_CTR

# Build and install portlibs
mkdir ${DEVKITPRO}/portlibs && mkdir ${PORTLIBS}/
git clone https://github.com/devkitPro/3ds_portlibs.git
cd 3ds_portlibs
make zlib
make install-zlib
make libogg
make install
make tremor
make install
cd ../
rm -rf 3ds_portlibs

# Build and install mpg123 (from fork)
git clone https://github.com/deltabeard/3ds_portlibs.git
cd 3ds_portlibs
make mpg123
cp mpg123-*/src/libmpg123/.libs/libmpg123.a ${DEVKITPRO}/portlibs/armv6k/lib/
cp mpg123-*/src/libmpg123/libmpg123.la ${DEVKITPRO}/portlibs/armv6k/lib/
cp mpg123-*/libmpg123.pc ${DEVKITPRO}/portlibs/armv6k/lib/pkgconfig/
cp mpg123-*/src/libmpg123/mpg123.h ${DEVKITPRO}/portlibs/armv6k/include/
cp mpg123-*/src/libmpg123/fmt123.h ${DEVKITPRO}/portlibs/armv6k/include/
cd ../
rm -rf 3ds_portlibs

# devkitArm
rm *.bz2
rm devkitARMupdate.pl
