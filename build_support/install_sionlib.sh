#!/bin/bash
set -euo pipefail

SIONLIBVERSION="1.7.4"
SIONLIB_INSTALL_PATH="${1:-${HOME}/.cache/sionlib.install}"

# Install SIONlib
wget --content-disposition "http://apps.fz-juelich.de/jsc/sionlib/download.php?version=${SIONLIBVERSION}"
tar xfz "sionlib-${SIONLIBVERSION}.tar.gz"
mv sionlib sionlib.src
pushd sionlib.src
./configure --prefix="${SIONLIB_INSTALL_PATH}" --mpi=openmpi --enable-python=3 --disable-fortran --disable-parutils --disable-mic
cd build-linux-gomp-openmpi
make -j"$(nproc)"
make install
popd || exit $?
rm -rf "sionlib-${SIONLIBVERSION}.tar.gz" sionlib.src
