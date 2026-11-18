#!/usr/bin/env bash
#
# container_check.sh
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

# Inside the container started by run_in_container.sh: install the tools,
# configure NEST and run check_iwyu.sh with the given arguments. Not meant to be
# called directly.

set -euo pipefail
cd /nest

export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
# IWYU 0.23 is built on clang 19, which has no OpenMP runtime package; clang 18
# only writes the compilation database and provides omp.h.
apt-get install -y -qq --no-install-recommends \
    iwyu clang-18 libomp-18-dev g++-15 make cmake git python3 python3-dev cython3 python3-pip \
    libopenmpi-dev openmpi-bin libgsl-dev libboost-dev libltdl-dev > /dev/null
git config --global --add safe.directory /nest

for arg in "$@"; do
    if [ "$arg" = "--fix" ]; then
        required="$(sed -n 's/^ *CLANG_REQUIRE_VERSION: *//p' .github/workflows/nestbuildmatrix.yml | head -n 1)"
        python3 -m pip install -q --break-system-packages "clang-format==$required"
    fi
done
include-what-you-use --version

BUILD_DIR=build/iwyu-container
omp_dir="$BUILD_DIR/omp-include"
mkdir -p "$omp_dir"
ln -sf "$(find /usr/lib/llvm-18 -name omp.h | head -n 1)" "$omp_dir/omp.h"
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_CXX_COMPILER=clang++-18 \
    -DCMAKE_CXX_FLAGS="--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15 -isystem /nest/$omp_dir" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_INSTALL_PREFIX="/nest/$BUILD_DIR/install" \
    -Dwith-openmp=ON -Dwith-mpi=ON -Dwith-gsl=ON -Dwith-boost=ON -Dwith-ltdl=ON > "$BUILD_DIR/cmake.log" 2>&1 ||
    { tail -n 30 "$BUILD_DIR/cmake.log" >&2; exit 2; }

# Options for check_iwyu.sh come first, then the build directory, then files.
opts=()
while [ $# -gt 0 ]; do
    case "$1" in
        --changed) opts+=("$1" "$2"); shift 2 ;;
        --fix) opts+=("$1"); shift ;;
        *) break ;;
    esac
done

rc=0
: > "$BUILD_DIR/summary.md"
GITHUB_STEP_SUMMARY="$BUILD_DIR/summary.md" build_support/check_iwyu.sh "${opts[@]}" "$BUILD_DIR" "$@" || rc=$?
# Files written in the container belong to root; give them back to the caller.
owner="${HOST_UID:-0}:${HOST_GID:-0}"
chown -R "$owner" build/iwyu-container
git diff --name-only -z | xargs -0 -r chown "$owner"
exit "$rc"
