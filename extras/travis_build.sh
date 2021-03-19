#!/bin/bash

# travis_build.sh
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


NEST_VPATH=build
mkdir -p "$NEST_VPATH/reports"

if [ "$xNEST_BUILD_TYPE" = "STATIC_CODE_ANALYSIS" ]; then
    echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
    echo "+               S T A T I C   C O D E   A N A L Y S I S                       +"
    echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"

    echo "MSGBLD0010: Initializing VERA++ static code analysis."
    wget --no-verbose https://bitbucket.org/verateam/vera/downloads/vera++-1.3.0.tar.gz
    tar -xzf vera++-1.3.0.tar.gz
    cd vera++-1.3.0
    cmake -DCMAKE_INSTALL_PREFIX=/usr -DVERA_LUA=OFF -DVERA_USE_SYSTEM_BOOST=ON
    sudo make install
    cd -
    rm -fr vera++-1.3.0 vera++-1.3.0.tar.gz
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
      # Note: BUG: Extracting the filenames may not work in all cases.
      #            The commit range might not properly reflect the history.
      #            see https://github.com/travis-ci/travis-ci/issues/2668
    if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
       echo "MSGBLD0080: PULL REQUEST: Retrieving changed files using git diff against $TRAVIS_BRANCH."
       file_names=`git diff --name-only --diff-filter=AM $TRAVIS_BRANCH...HEAD`
    else
       echo "MSGBLD0090: Retrieving changed files using git diff in range $TRAVIS_COMMIT_RANGE."
       file_names=`git diff --name-only $TRAVIS_COMMIT_RANGE`
    fi

    # Note: uncomment the following line to static check *all* files, not just those that have changed.
    # Warning: will run for a very long time (will time out on Travis CI instances)

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
    PEP8=pep8

    # Perform or skip a certain analysis.
    PERFORM_VERA=true
    PERFORM_CPPCHECK=true
    PERFORM_CLANG_FORMAT=true
    PERFORM_PEP8=true

    # The following command line parameters indicate whether static code analysis error messages
    # will cause the Travis CI build to fail or are ignored.
    IGNORE_MSG_VERA=false
    IGNORE_MSG_CPPCHECK=true
    IGNORE_MSG_CLANG_FORMAT=false
    IGNORE_MSG_PEP8=false

    # The script is called within the Travis CI environment and thus can not be run incremental.
    RUNS_ON_TRAVIS=true
    INCREMENTAL=false

    chmod +x extras/static_code_analysis.sh
    ./extras/static_code_analysis.sh "$RUNS_ON_TRAVIS" "$INCREMENTAL" "$file_names" "$NEST_VPATH" \
    "$VERA" "$CPPCHECK" "$CLANG_FORMAT" "$PEP8" \
    "$PERFORM_VERA" "$PERFORM_CPPCHECK" "$PERFORM_CLANG_FORMAT" "$PERFORM_PEP8" \
    "$IGNORE_MSG_VERA" "$IGNORE_MSG_CPPCHECK" "$IGNORE_MSG_CLANG_FORMAT" "$IGNORE_MSG_PEP8"

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

NEST_RESULT=result
if [ "$(uname -s)" = 'Linux' ]; then
    NEST_RESULT=$(readlink -f $NEST_RESULT)
else
    NEST_RESULT=$(greadlink -f $NEST_RESULT)
fi
mkdir "$NEST_RESULT"

echo "MSGBLD0340: Build completed."
