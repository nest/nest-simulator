#!/bin/bash
set -euo pipefail

NEUROSIM_INSTALL_PATH="${1:-${HOME}/.cache/libneurosim.install}"
CSA_INSTALL_PATH="${1:-${HOME}/.cache/csa.install}"
PYLIB_DIR="$(python3 -c "import sysconfig; print(sysconfig.get_path('include'))" | sed 's/include/lib/')" || true  # was "$1" before

# Install libneurosim
git clone --branch v1.2.0 --depth 1 "https://github.com/INCF/libneurosim.git" libneurosim.src
pushd libneurosim.src
./autogen.sh
./configure --prefix="${NEUROSIM_INSTALL_PATH}" --with-mpi --with-python=3
make -j"$(nproc)"
make install
popd || exit $?
rm -rf libneurosim.src

# Install csa
git clone --branch v0.1.13 --depth 1 "https://github.com/INCF/csa.git" csa.src
pushd csa.src
sed -i 's/lpyneurosim/lpy3neurosim/g' configure.ac
sed -i 's/print __version__/print\(__version__\)/g' Makefile.am
./autogen.sh
LDFLAGS="${PYLIB_DIR:+-L${PYLIB_DIR}}" ./configure --with-libneurosim="${NEUROSIM_INSTALL_PATH}" --prefix="${CSA_INSTALL_PATH}"
make
make install
popd || exit $?
rm -rf csa.src
