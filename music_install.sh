#!/bin/bash

 # Install current MUSIC version.
git clone https://github.com/INCF/MUSIC.git music
cd music
./autogen.sh
./configure --prefix=$HOME/.cache/music.install --PYTHON_INCLUDE=/opt/python/3.5-dev/include/python3.5m/Python.h
make
make install
cd ..
rm -rf music