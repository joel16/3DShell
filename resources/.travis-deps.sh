#!/bin/sh
set -e
set -x

# Build and install devkitARM + ctrulib
wget https://sourceforge.net/projects/devkitpro/files/Automated%20Installer/devkitARMupdate.pl
git clone https://github.com/smealum/ctrulib.git
perl devkitARMupdate.pl

# Get latest ctrulib and overwrite bundled one
cd ctrulib/libctru && make ; cd -
cp -rf ctrulib/libctru/ ${DEVKITPRO}

# Get latest version of Citro3D
git clone --recursive https://github.com/fincs/citro3d.git
cd citro3d
make install
cd ../
rm -rf citro3d

# Install latest version of picasso
#git clone https://github.com/fincs/picasso.git
#cd picasso
#./autogen.sh
#./configure
#make
#sudo cp picasso ${DEVKITPRO}/devkitARM/bin/picasso
#cd ../
#rm -rf picasso

# Build and install bannertool
git clone --recursive https://github.com/Steveice10/bannertool.git
cd bannertool
make clean && make
sudo cp output/linux-x86_64/bannertool ${DEVKITPRO}/devkitARM/bin/bannertool
cd ..
rm -rf bannertool

# Build and install makerom
git clone https://github.com/profi200/Project_CTR.git
cd Project_CTR/makerom
make clean && make
sudo cp makerom ${DEVKITPRO}/devkitARM/bin/makerom
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

cd -
