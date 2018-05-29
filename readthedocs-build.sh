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

# Set the NEST CMake-build configuration according to the build matrix in '.travis.yml'.

CONFIGURE_THREADING="-Dwith-openmp=OFF"
CONFIGURE_MPI="-Dwith-mpi=OFF"
CONFIGURE_PYTHON="-Dwith-python=ON"
CONFIGURE_MUSIC="-Dwith-music=OFF"
CONFIGURE_GSL="-Dwith-gsl=OFF"
CONFIGURE_LTDL="-Dwith-ltdl=OFF"
CONFIGURE_READLINE="-Dwith-readline=OFF"
CONFIGURE_LIBNEUROSIM="-Dwith-libneurosim=OFF"

NEST_VPATH=build
NEST_RESULT=result
NEST_RESULT=$(readlink -f $NEST_RESULT)

rm -rf "$NEST_VPATH" "$NEST_RESULT"
mkdir "$NEST_VPATH" "$NEST_RESULT"
mkdir "$NEST_VPATH/reports"



echo
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               C O N F I G U R E   N E S T   B U I L D                       +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0230: Configuring CMake."
cmake \
  -DCMAKE_INSTALL_PREFIX="$NEST_RESULT" \
  -Dwith-optimize=ON \
  -Dwith-warning=ON \
  $CONFIGURE_THREADING \
  $CONFIGURE_MPI \
  $CONFIGURE_PYTHON \
  $CONFIGURE_MUSIC \
  $CONFIGURE_GSL \
  $CONFIGURE_LTDL \
  $CONFIGURE_READLINE \
  $CONFIGURE_LIBNEUROSIM \
  ..
echo "MSGBLD0240: CMake configure completed."

echo
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               B U I L D   N E S T                                           +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0250: Running Make."
make VERBOSE=1
echo "MSGBLD0260: Make completed."

echo
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               I N S T A L L   N E S T                                       +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0270: Running make install."
make install
echo "MSGBLD0280: Make install completed."

echo
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               R U N   N E S T   T E S T S U I T E                           +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0290: Running make installcheck."
# export PYTHONPATH=$HOME/.cache/csa.install/lib/python2
# .7/site-packages:$PYTHONPATH
# export LD_LIBRARY_PATH=$HOME/.cache/csa.install/lib:$LD_LIBRARY_PATH
# make installcheck
# echo "MSGBLD0300: Make installcheck completed."


echo "MSGBLD0340: Build completed."

ls -l

cd ..

. result/bin/nest_vars.sh