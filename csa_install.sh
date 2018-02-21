#!/bin/bash                                                                           
 # Install current csa version.                                                     
# Install csa
  - git clone https://github.com/INCF/csa.git
  - pushd csa
  - ./autogen.sh
  - ./configure --with-libneurosim=$HOME/.cache/libneurosim.install --prefix=$HOME/.c\
ache/csa.install
  - make
  - make install
  - popd





git clone https://github.com/INCF/MUSIC.git music
cd music
./autogen.sh
./configure --prefix=$HOME/.cache/music.install
make
make install
cd ..
rm -rf music

