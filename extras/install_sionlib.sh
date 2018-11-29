#!/bin/bash

# Install SIONlib
wget --content-disposition http://apps.fz-juelich.de/jsc/sionlib/download.php?version=1.6.2
tar xfz sionlib-1.6.2.tar.gz
mv sionlib sionlib.src
pushd sionlib.src
./configure --prefix=$HOME/.cache/sionlib.install --mpi=openmpi --enable-python=2 --disable-fortran --disable-parutils --disable-mic
cd build-linux-gomp-openmpi
make
make install
popd
rm -rf sionlib-1.6.2.tar.gz sionlib.src
