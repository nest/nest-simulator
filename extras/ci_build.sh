#!/bin/bash

# ci_build.sh
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


# This shell script is part of the NEST CI build and test environment.
# It is invoked by the top-level Github Actions script '.github/workflows/nestbuildmatrix.yml'.
#
# NOTE: This shell script is tightly coupled to 'extras/parse_build_log.py'.
#       Any changes to message numbers (MSGBLDnnnn) or the variable name
#      'file_names' have effects on the build/test-log parsing process.


# Exit shell if any subcommand or pipline returns a non-zero status.
set -ex

env
if [ "$xNEST_BUILD_COMPILER" = "CLANG" ]; then
    export CC=clang-11
    export CXX=clang++-11
fi

NEST_VPATH=build
mkdir -p "$NEST_VPATH/reports"

if [ "$xNEST_BUILD_TYPE" = "STATIC_CODE_ANALYSIS" ]; then
    echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
    echo "+               S T A T I C   C O D E   A N A L Y S I S                       +"
    echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"

    echo "MSGBLD0010: Initializing VERA++ static code analysis."
    export PYTHON_EXECUTABLE="$(which python3)"
    export PYTHON_INCLUDE_DIR="`python3 -c "import sysconfig; print(sysconfig.get_path('include'))"`"
    export PYLIB_BASE="lib`basename $PYTHON_INCLUDE_DIR`"
    export PYLIB_DIR="$(dirname `sed 's/include/lib/' <<< $PYTHON_INCLUDE_DIR`)"
    export PYTHON_LIBRARY="`find $PYLIB_DIR \( -name $PYLIB_BASE.so -o -name $PYLIB_BASE.dylib \) -print -quit`"
    echo "--> Detected PYTHON_LIBRARY=$PYTHON_LIBRARY"
    echo "--> Detected PYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR"
    CONFIGURE_PYTHON="-DPYTHON_EXECUTABLE=$PYTHON_EXECUTABLE -DPYTHON_LIBRARY=$PYTHON_LIBRARY -DPYTHON_INCLUDE_DIR=$PYTHON_INCLUDE_DIR"
    # Add the NEST profile to the VERA++ profiles.
    sudo cp extras/vera++.profile /usr/lib/vera++/profiles/nest
    echo "MSGBLD0020: VERA++ initialization completed."
    if [ ! -f "$HOME/.cache/bin/cppcheck" ]; then
        echo "MSGBLD0030: Installing CPPCHECK version 1.69."
        # Build cppcheck version 1.69
        git clone https://github.com/danmar/cppcheck.git
        cd cppcheck
        git checkout tags/1.69
        mkdir -p install
        make PREFIX=$HOME/.cache CFGDIR=$HOME/.cache/cfg HAVE_RULES=yes install
        cd -
        echo "MSGBLD0040: CPPCHECK installation completed."

        echo "MSGBLD0050: Installing CLANG-FORMAT."
        wget --no-verbose http://llvm.org/releases/3.6.2/clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz
        tar xf clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz
        # Copy and not move because '.cache' may aleady contain other subdirectories and files.
        cp -R clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04/* $HOME/.cache
        echo "MSGBLD0060: CLANG-FORMAT installation completed."

        # Remove these directories, otherwise the copyright-header check will complain.
        rm -rf cppcheck clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04
    fi

    # Ensure that the cppcheck and clang-format installation can be found.
    export PATH=$HOME/.cache/bin:$PATH

    echo "MSGBLD0070: Retrieving changed files."
    file_names=$CHANGED_FILES
    echo "MSGBLD0071: $file_names"

    # Note: uncomment the following line to static check *all* files, not just those that have changed.
    # Warning: will run for a very long time

    # file_names=`find . -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.py"`

    for single_file_name in $file_names
    do
        echo "MSGBLD0095: File changed: $single_file_name"
    done
    echo "MSGBLD0100: Retrieving changed files completed."
    echo

    # Set the command line arguments for the static code analysis script and execute it.

    # The names of the static code analysis tools executables.
    VERA=vera++
    CPPCHECK=cppcheck
    CLANG_FORMAT=clang-format
    PEP8=pycodestyle
    PYCODESTYLE_IGNORES="E121,E123,E126,E226,E24,E704,W503,W504"

    # Perform or skip a certain analysis.
    PERFORM_VERA=true
    PERFORM_CPPCHECK=true
    PERFORM_CLANG_FORMAT=true
    PERFORM_PEP8=true

    # The following command line parameters indicate whether static code analysis error messages
    # will cause the CI build to fail or are ignored.
    IGNORE_MSG_VERA=false
    IGNORE_MSG_CPPCHECK=true
    IGNORE_MSG_CLANG_FORMAT=false
    IGNORE_MSG_PYCODESTYLE=false

    # The script is called within the CI environment and thus can not be run incremental.
    RUNS_ON_CI=true
    INCREMENTAL=false

    chmod +x extras/static_code_analysis.sh
    ./extras/static_code_analysis.sh "$RUNS_ON_CI" "$INCREMENTAL" "$file_names" "$NEST_VPATH" \
    "$VERA" "$CPPCHECK" "$CLANG_FORMAT" "$PEP8" \
    "$PERFORM_VERA" "$PERFORM_CPPCHECK" "$PERFORM_CLANG_FORMAT" "$PERFORM_PEP8" \
    "$IGNORE_MSG_VERA" "$IGNORE_MSG_CPPCHECK" "$IGNORE_MSG_CLANG_FORMAT" "$IGNORE_MSG_PYCODESTYLE" \
    "$PYCODESTYLE_IGNORES"

    exit $?
fi

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
# '.github/workflows/nestbuildmatrix.yml'.

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
    export PYTHON_INCLUDE_DIR=`python3 -c "import sysconfig; print(sysconfig.get_path('include'))"`
    export PYLIB_BASE=lib`basename $PYTHON_INCLUDE_DIR`
    export PYLIB_DIR=$(dirname `sed 's/include/lib/' <<< $PYTHON_INCLUDE_DIR`)
    export PYTHON_LIBRARY=`find $PYLIB_DIR \( -name $PYLIB_BASE.so -o -name $PYLIB_BASE.dylib \) -print -quit`
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
    #CONFIGURE_BOOST="-Dwith-boost=$HOME/.cache/boost_1_72_0.install"
    CONFIGURE_BOOST="-Dwith-boost=$HOME/.cache/boost_1_71_0.install"
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
#if [[ "$OSTYPE" = darwin* ]] ; then
    #sed -i -e 's/mpirun -np/mpirun --oversubscribe -np/g' ~/.nestrc
#fi
sed -i -e 's/mpirun -np/mpirun --oversubscribe -np/g' ~/.nestrc
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
echo "MSGBLD0340: Build completed."
