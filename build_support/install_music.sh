#!/bin/bash
set -euo pipefail

# Install current MUSIC version.
git clone "https://github.com/INCF/MUSIC.git" music.src
pushd music.src
./autogen.sh
./configure --prefix="${HOME}/.cache/music.install"
make
make install
popd || exit $?
rm -rf music.src
