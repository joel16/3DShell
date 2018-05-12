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
git clone --recursive https://github.com/Steveice10/bannertool
cd bannertool/
make
cp output/linux-x86_64/bannertool ${DEVKITPRO}/devkitARM/bin/bannertool
cd ../
rm -rf bannertool/

# Build and install makerom
git clone https://github.com/profi200/Project_CTR.git
cd Project_CTR/makerom
make
cp makerom ${DEVKITPRO}/devkitARM/bin/makerom
cd ../..
rm -rf Project_CTR

# Build and install portlibs using pacman
mkdir ${DEVKITPRO}/portlibs && mkdir ${PORTLIBS}/
wget https://github.com/devkitPro/pacman/releases/download/v1.0.0/devkitpro-pacman.deb
sudo dpkg -i devkitpro-pacman.deb

# Remove mbedtls if it's included - causes issues.
sudo rm -rf ${DEVKITPRO}/portlibs/3ds/include/mbedtls/
sudo rm ${DEVKITPRO}/portlibs/3ds/lib/libmbedcrypto.a
sudo rm ${DEVKITPRO}/portlibs/3ds/lib/libmbedtls.a 
sudo rm ${DEVKITPRO}/portlibs/3ds/lib/libmbedx509.a

# Install required libs
sudo dkp-pacman -S 3ds-zlib --noconfirm
sudo dkp-pacman -S 3ds-curl --noconfirm
sudo dkp-pacman -S 3ds-mpg123 --noconfirm
sudo dkp-pacman -S 3ds-libogg --noconfirm
sudo dkp-pacman -S 3ds-libvorbisidec --noconfirm

# Remove unnecessary files
rm *.bz2
rm devkitARMupdate.pl
rm devkitpro-pacman.deb