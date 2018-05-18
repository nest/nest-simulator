#!/bin/bash

# build.sh
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
# NOTE: This shell script is tightly coupled to Python script
#       'extras/parse_travis_log.py'. 
#       Any changes to message numbers (MSGBLDnnnn) or the variable name
#      'file_names' have effects on the build/test-log parsing process.


# Exit shell if any subcommand or pipline returns a non-zero status.
set -e

mkdir -p $HOME/.matplotlib
cat > $HOME/.matplotlib/matplotlibrc <<EOF
    backend : svg
EOF

# Set the NEST CMake-build configuration according to the build matrix in '.travis.yml'.
if [ "$xTHREADING" = "1" ] ; then
    CONFIGURE_THREADING="-Dwith-openmp=ON"
else
    CONFIGURE_THREADING="-Dwith-openmp=OFF"
fi

if [ "$xMPI" = "1" ] ; then
    CONFIGURE_MPI="-Dwith-mpi=ON"
else
    CONFIGURE_MPI="-Dwith-mpi=OFF"
fi

if [ "$xPYTHON" = "1" ] ; then
    CONFIGURE_PYTHON="-Dwith-python=ON"
else
    CONFIGURE_PYTHON="-Dwith-python=OFF"
fi

if [ "$xMUSIC" = "1" ] ; then
    CONFIGURE_MUSIC="-Dwith-music=$HOME/.cache/music.install"
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

if [ "$xLIBNEUROSIM" = "1" ] ; then
    CONFIGURE_LIBNEUROSIM="-Dwith-libneurosim=$HOME/.cache/libneurosim.install"
else
    CONFIGURE_LIBNEUROSIM="-Dwith-libneurosim=OFF"
fi


NEST_VPATH=build
NEST_RESULT=result
NEST_RESULT=$(readlink -f $NEST_RESULT)

mkdir "$NEST_VPATH" "$NEST_RESULT"
mkdir "$NEST_VPATH/reports"

if [ "$xSTATIC_ANALYSIS" = "1" ] ; then
  echo
  echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
  echo "+               S T A T I C   C O D E   A N A L Y S I S                       +"
  echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"

  echo "MSGBLD0010: Initializing VERA++ static code analysis."
  # Add the NEST profile to the VERA++ profiles.
  sudo cp ./extras/vera++.profile /usr/lib/vera++/profiles/nest
  echo "MSGBLD0020: VERA++ initialization completed."  

  if [ ! -f "$HOME/.cache/bin/cppcheck" ]; then
    echo "MSGBLD0030: Installing CPPCHECK version 1.69."
    # Build cppcheck version 1.69.
    git clone https://github.com/danmar/cppcheck.git
    cd cppcheck
    git checkout tags/1.69
    mkdir -p install
    make PREFIX=$HOME/.cache CFGDIR=$HOME/.cache/cfg HAVE_RULES=yes install
    cd ..
    echo "MSGBLD0040: CPPCHECK installation completed."

    echo "MSGBLD0050: Installing CLANG-FORMAT."
    wget --no-verbose http://llvm.org/releases/3.6.2/clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz
    tar xf clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz
    # Copy and not move because '.cache' may aleady contain other subdirectories and files.
    cp -R clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04/* $HOME/.cache
    echo "MSGBLD0060: CLANG-FORMAT installation completed."

    # Remove these directories, otherwise the copyright-header check will complain.
    rm -rf ./cppcheck
    rm -rf ./clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04
  fi

  # Ensure that the cppcheck and clang-format installation can be found.
  export PATH=$HOME/.cache/bin:$PATH
  
  echo "MSGBLD0070: Retrieving changed files."
  # Note: BUG: Extracting the filenames may not work in all cases. 
  #            The commit range might not properly reflect the history.
  #            see https://github.com/travis-ci/travis-ci/issues/2668
  if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    echo "MSGBLD0080: PULL REQUEST: Retrieving changed files using GitHub API."
    file_names=`curl "https://api.github.com/repos/$TRAVIS_REPO_SLUG/pulls/$TRAVIS_PULL_REQUEST/files" | jq '.[] | .filename' | tr '\n' ' ' | tr '"' ' '`
  else
    echo "MSGBLD0090: Retrieving changed files using git diff."    
    file_names=`(git diff --name-only $TRAVIS_COMMIT_RANGE || echo "") | tr '\n' ' '`
  fi

  printf '%s\n' "$file_names" | while IFS= read -r line
  do
    for single_file_name in $file_names
    do
      echo "MSGBLD0095: File changed: $single_file_name"
    done
  done
  echo "MSGBLD0100: Retrieving changed files completed."
  echo


  # Set the command line arguments for the static code analysis script and execute it.

  # The names of the static code analysis tools executables.
  VERA=vera++                   
  CPPCHECK=cppcheck
  CLANG_FORMAT=clang-format
  PEP8=pep8

  # Perform or skip a certain analysis.
  PERFORM_VERA=true
  PERFORM_CPPCHECK=true
  PERFORM_CLANG_FORMAT=true
  PERFORM_PEP8=true

  # The following command line parameters indicate whether static code analysis error messages
  # will cause the Travis CI build to fail or are ignored.
  IGNORE_MSG_VERA=true
  IGNORE_MSG_CPPCHECK=true
  IGNORE_MSG_CLANG_FORMAT=false
  IGNORE_MSG_PEP8=false

  # The script is called within the Travis CI environment and cannot be run incremental.
  RUNS_ON_TRAVIS=true
  INCREMENTAL=false

  sudo chmod +x ./extras/static_code_analysis.sh
  ./extras/static_code_analysis.sh "$RUNS_ON_TRAVIS" "$INCREMENTAL" "$file_names" "$NEST_VPATH" \
  "$VERA" "$CPPCHECK" "$CLANG_FORMAT" "$PEP8" \
  "$PERFORM_VERA" "$PERFORM_CPPCHECK" "$PERFORM_CLANG_FORMAT" "$PERFORM_PEP8" \
  "$IGNORE_MSG_VERA" "$IGNORE_MSG_CPPCHECK" "$IGNORE_MSG_CLANG_FORMAT" "$IGNORE_MSG_PEP8"

fi   # Static code analysis.


cd "$NEST_VPATH"
cp ../examples/sli/nestrc.sli ~/.nestrc

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

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  echo "MSGBLD0310: This build was triggered by a pull request."
  echo "MSGBLD0330: (WARNING) Build artifacts not uploaded to Amazon S3."
fi

if [ "$TRAVIS_REPO_SLUG" != "nest/nest-simulator" ] ; then
  echo "MSGBLD0320: This build was from a forked repository and not from nest/nest-simulator."
  echo "MSGBLD0330: (WARNING) Build artifacts not uploaded to Amazon S3."
fi

echo "MSGBLD0340: Build completed."

ls -l

cd ..

. result/bin/nest_vars.sh
