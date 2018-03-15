#!/bin/bash                                                                           
# Install libneurosim
git clone https://github.com/INCF/libneurosim.git libneurosim
cd libneurosim
./autogen.sh
./configure --prefix=$HOME/.cache/libneurosim.install --with-mpi --PYTHON_INCLUDE=/opt/python/3.5-dev/include/python3.5m/Python.h
make
make install
cd ..
rm -rf libneurosim

# Install csa
git clone https://github.com/INCF/csa.git csa
cd csa
./autogen.sh
./configure --with-libneurosim=$HOME/.cache/libneurosim.install --prefix=$HOME/.cache/csa.install
make
make install
cd ..
rm -rf csa
