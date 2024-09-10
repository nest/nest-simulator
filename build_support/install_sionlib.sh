#!/bin/bash

SIONLIBVERSION="1.7.4"

# Install SIONlib
wget --content-disposition http://apps.fz-juelich.de/jsc/sionlib/download.php?version=${SIONLIBVERSION}
tar xfz sionlib-${SIONLIBVERSION}.tar.gz
mv sionlib sionlib.src
pushd sionlib.src
./configure --prefix=$HOME/.cache/sionlib.install --mpi=openmpi --enable-python=3 --disable-fortran --disable-parutils --disable-mic
cd build-linux-gomp-openmpi
make
make install
popd
rm -rf sionlib-${SIONLIBVERSION}.tar.gz sionlib.src
