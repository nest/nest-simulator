#!/bin/bash                                                                           
 # Install current csa version.                                                     
# Install csa
  - git clone https://github.com/INCF/csa.git
  - cd csa
  - git checkout tags/v0.1.8
  - ./autogen.sh
  - ./configure --with-libneurosim=$HOME/.cache/libneurosim.install --prefix=$HOME/.cache/csa.install
  - make
  - make install
  - cd ..

