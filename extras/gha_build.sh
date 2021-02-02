#!/bin/bash

# gha_build.sh
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


# This shell script is part of the NEST Travis CI build and test environment.
# It is invoked by the top-level Travis script '.travis.yml'.
#
# NOTE: This shell script is tightly coupled to 'extras/parse_travis_log.py'.
#       Any changes to message numbers (MSGBLDnnnn) or the variable name
#      'file_names' have effects on the build/test-log parsing process.


# Exit shell if any subcommand or pipline returns a non-zero status.
set -e


if [ "$xNEST_BUILD_COMPILER" = "CLANG" ]; then
    export CC=clang-7
    export CXX=clang++-7
fi

NEST_VPATH=build
mkdir -p "$NEST_VPATH/reports"

echo
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               C O N F I G U R E   N E S T   B U I L D                       +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0230: Reading build type."

# This defines the base settings of all build options off. The base
# settings are also used for NEST_BUILD_TYPE=MINIMAL, which is not
# explicitly checked for below.

xGSL=0
xLIBBOOST=0
xLIBNEUROSIM=0
xLTDL=0
xMPI=0
xMUSIC=0
xOPENMP=0
xPYTHON=0
xREADLINE=0
xSIONLIB=0

CXX_FLAGS="-pedantic -Wextra -Wno-unknown-pragmas"

if [ "$xNEST_BUILD_TYPE" = "OPENMP_ONLY" ]; then
    xGSL=1
    xLIBBOOST=1
    xLTDL=1
    xOPENMP=1
    CXX_FLAGS="-pedantic -Wextra"
fi

if [ "$xNEST_BUILD_TYPE" = "MPI_ONLY" ]; then
    xGSL=1
    xLIBBOOST=1
    xLTDL=1
    xMPI=1
fi

if [ "$xNEST_BUILD_TYPE" = "FULL" ]; then
    xGSL=1
    xLIBBOOST=1
    xLIBNEUROSIM=1
    xLTDL=1
    xMPI=1
    xMUSIC=1
    xOPENMP=1
    xPYTHON=1
    xREADLINE=1
    xSIONLIB=1
    CXX_FLAGS="-pedantic -Wextra"
fi

if [ "$xNEST_BUILD_TYPE" = "FULL_NO_EXTERNAL_FEATURES" ]; then
    xGSL=1
    xLIBBOOST=1
    xLIBNEUROSIM=0
    xLTDL=1
    xMPI=0
    xMUSIC=0
    xOPENMP=1
    xPYTHON=1
    xREADLINE=1
    xSIONLIB=0
fi

echo "MSGBLD0232: Setting configuration variables."

# Set the NEST CMake-build configuration according to the variables
# set above based on the ones set in the build stage matrix in
# '.travis.yml'.

if [ "$xOPENMP" = "1" ] ; then
    CONFIGURE_OPENMP="-Dwith-openmp=ON"
else
    CONFIGURE_OPENMP="-Dwith-openmp=OFF"
fi

if [ "$xMPI" = "1" ] ; then
    CONFIGURE_MPI="-Dwith-mpi=ON"
else
    CONFIGURE_MPI="-Dwith-mpi=OFF"
fi

if [ "$xPYTHON" = "1" ] ; then
    PYTHON_INCLUDE_DIR=`python3 -c "import sysconfig; print(sysconfig.get_path('include'))"`
    PYLIB_BASE=lib`basename $PYTHON_INCLUDE_DIR`
    PYLIB_DIR=$(dirname `sed 's/include/lib/' <<< $PYTHON_INCLUDE_DIR`)
    PYTHON_LIBRARY=`find $PYLIB_DIR \( -name $PYLIB_BASE.so -o -name $PYLIB_BASE.dylib \) -print -quit`
    echo "--> Detected PYTHON_LIBRARY=$PYTHON_LIBRARY"
    echo "--> Detected PYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR"
    CONFIGURE_PYTHON="-DPYTHON_LIBRARY=$PYTHON_LIBRARY -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR"
    mkdir -p $HOME/.matplotlib
    echo "backend : svg" > $HOME/.matplotlib/matplotlibrc
else
    CONFIGURE_PYTHON="-Dwith-python=OFF"
fi
if [ "$xMUSIC" = "1" ] ; then
    CONFIGURE_MUSIC="-Dwith-music=$HOME/.cache/music.install"
    chmod +x extras/install_music.sh
    ./extras/install_music.sh
else
    CONFIGURE_MUSIC="-Dwith-music=OFF"
fi
if [ "$xGSL" = "1" ] ; then
    CONFIGURE_GSL="-Dwith-gsl=ON"
else
    CONFIGURE_GSL="-Dwith-gsl=OFF"
fi
if [ "$xLTDL" = "1" ] ; then
    CONFIGURE_LTDL="-Dwith-ltdl=ON"
else
    CONFIGURE_LTDL="-Dwith-ltdl=OFF"
fi
if [ "$xREADLINE" = "1" ] ; then
    CONFIGURE_READLINE="-Dwith-readline=ON"
else
    CONFIGURE_READLINE="-Dwith-readline=OFF"
fi
if [ "$xLIBBOOST" = "1" ] ; then
    CONFIGURE_BOOST="-Dwith-boost=$HOME/.cache/boost_1_72_0.install"
    chmod +x extras/install_libboost.sh
    ./extras/install_libboost.sh
else
    CONFIGURE_BOOST="-Dwith-boost=OFF"
fi
if [ "$xSIONLIB" = "1" ] ; then
    CONFIGURE_SIONLIB="-Dwith-sionlib=$HOME/.cache/sionlib.install"
    chmod +x extras/install_sionlib.sh
    ./extras/install_sionlib.sh
else
    CONFIGURE_SIONLIB="-Dwith-sionlib=OFF"
fi
if [ "$xLIBNEUROSIM" = "1" ] ; then
    CONFIGURE_LIBNEUROSIM="-Dwith-libneurosim=$HOME/.cache/libneurosim.install"
    chmod +x extras/install_csa-libneurosim.sh
    ./extras/install_csa-libneurosim.sh $PYLIB_DIR
    PYMAJOR=`python3 -c 'import sys; print("%i.%i" % sys.version_info[:2])'`
    export PYTHONPATH=$HOME/.cache/csa.install/lib/python$PYMAJOR/site-packages${PYTHONPATH:+:$PYTHONPATH}
    if [[ $OSTYPE == darwin* ]]; then
        export DYLD_LIBRARY_PATH=$HOME/.cache/csa.install/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}
    else
        export LD_LIBRARY_PATH=$HOME/.cache/csa.install/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
    fi
else
    CONFIGURE_LIBNEUROSIM="-Dwith-libneurosim=OFF"
fi
cp extras/nestrc.sli ~/.nestrc
# Explicitly allow MPI oversubscription. This is required by Open MPI versions > 3.0.
# Not having this in place leads to a "not enough slots available" error.
if [[ "$OSTYPE" = darwin* ]] ; then
    sed -i -e 's/mpirun -np/mpirun --oversubscribe -np/g' ~/.nestrc
fi
NEST_RESULT=result
if [ "$(uname -s)" = 'Linux' ]; then
    NEST_RESULT=$(readlink -f $NEST_RESULT)
else
    NEST_RESULT=$(greadlink -f $NEST_RESULT)
fi
mkdir "$NEST_RESULT"
echo "MSGBLD0235: Running CMake."
cd "$NEST_VPATH"
cmake \
    -DCMAKE_INSTALL_PREFIX="$NEST_RESULT" \
    -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
    -Dwith-optimize=ON \
    -Dwith-warning=ON \
    $CONFIGURE_BOOST \
    $CONFIGURE_OPENMP \
    $CONFIGURE_MPI \
    $CONFIGURE_PYTHON \
    $CONFIGURE_MUSIC \
    $CONFIGURE_GSL \
    $CONFIGURE_LTDL \
    $CONFIGURE_READLINE \
    $CONFIGURE_SIONLIB \
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
make installcheck
echo "MSGBLD0300: Make installcheck completed."

fi
echo "MSGBLD0340: Build completed."
