#!/bin/bash                                                                           
# Install libneurosim
git clone https://github.com/INCF/libneurosim.git libneurosim.src
pushd libneurosim.src
./autogen.sh
./configure --prefix=$HOME/.cache/libneurosim.install --with-mpi --with-python=3
make
make install
popd
rm -rf libneurosim.src

# Install csa
git clone https://github.com/INCF/csa.git csa.src
pushd csa.src
sed -i 's/lpyneurosim/lpy3neurosim/g' configure.ac
sed -i 's/print __version__/print\(__version__\)/g' Makefile.am
./autogen.sh
LDFLAGS=-L$1 ./configure --with-libneurosim=$HOME/.cache/libneurosim.install --prefix=$HOME/.cache/csa.install
make
make install
popd
rm -rf csa.src
