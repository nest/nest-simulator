#!/usr/bin/env bash
set -euo pipefail

MUSIC_INSTALL_PATH="${1:-${HOME}/.cache/music.install}"
# Install current MUSIC version.
git clone "https://github.com/INCF/MUSIC.git" music.src
pushd music.src
./autogen.sh
./configure --prefix="${MUSIC_INSTALL_PATH}"
make -j"$(nproc)"
make install
popd || exit $?
rm -rf music.src
