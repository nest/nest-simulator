#!/bin/bash

# Install current MUSIC version.
git clone https://github.com/INCF/MUSIC.git music.src
pushd music.src
./autogen.sh
./configure --prefix=$HOME/.cache/music.install
make
make install
popd
rm -rf music.src
