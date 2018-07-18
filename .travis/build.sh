#!/bin/bash -ex

source /etc/profile.d/devkit-env.sh
cd /3DShell
make clean && make
rm .gitignore .gitmodules .travis.yml 3DShell.elf 3DShell.smdh devkitpro-pacman.deb LICENSE Makefile README.md
rm -rf .travis build include res romfs source