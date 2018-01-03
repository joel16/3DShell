#!/bin/sh
set -e
set -x

make clean && make
mv out/3DShell.cia .
mv out/3DShell.3dsx .
rm .gitignore .gitmodules .travis.yml LICENSE Makefile README.md
rm -rf build include out romfs resources source