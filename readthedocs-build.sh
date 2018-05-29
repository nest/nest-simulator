#!/bin/bash

# readthedocs-build.sh
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


# Exit shell if any subcommand or pipline returns a non-zero status.
set -e

mkdir -p $HOME/.matplotlib
cat > $HOME/.matplotlib/matplotlibrc <<EOF
    backend : svg
EOF


mkdir ./nest-build
mkdir ./nest-install
cd ./nest-build
cmake -DCMAKE_INSTALL_PREFIX:PATH=./nest-install \
    -Dwith-python:=3 \
    -Dwith-mpi:BOOL=OFF \
    -Dwith-gsl:BOOL=OFF \
    -Dwith-libneurosim:BOOL=OFF \
    -Dwith-music:BOOL=OFF \
    ../nest-2.14.0
make
make install

ls -l

cd ..

. ./nest-install/bin/nest_vars.sh
