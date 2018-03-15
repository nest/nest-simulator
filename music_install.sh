#!/bin/bash

 # Install current MUSIC version.
git clone https://github.com/INCF/MUSIC.git music
cd music
./autogen.sh
./configure --prefix=$HOME/.cache/music.install
make
make install
cd ..
rm -rf music