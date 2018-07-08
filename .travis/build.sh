#!/bin/bash -ex

source /etc/profile.d/devkit-env.sh
cd /3DShell
make clean && make
rm .gitignore .gitmodules .travis.yml LICENSE Makefile README.md
rm -rf .travis build include res romfs source