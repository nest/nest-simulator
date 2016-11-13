#!/bin/sh

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
# NOTE: This shell script is pysically coupled to python script
#       'extras/parse_travis_log.py'. 
#       Any changes to message numbers (MSGBLDnnnn) or the variable name
#      'file_names' have effects on the build/test-log parsing process.


# Exit shell if any subcommand or pipline returns a non-zero status.
set -e

# Uncomment this shell-option for debugging.
# set -x

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

NEST_VPATH=build
NEST_RESULT=result
mkdir "$NEST_VPATH" "$NEST_RESULT"
mkdir "$NEST_VPATH/reports"
NEST_RESULT=$(readlink -f $NEST_RESULT)

if [ "$xSTATIC_ANALYSIS" = "1" ] ; then
  echo "\n"
  echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
  echo "+               S T A T I C   C O D E   A N A L Y S I S                       +"
  echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"

  echo "MSGBLD0010: Initializing VERA++ static code analysis."
  # Create a VERA++ profile for NEST in './vera_home' for static code analysis.
  mkdir -p vera_home
  cp -r /usr/lib/vera++/* ./vera_home
  cat > ./vera_home/profiles/nest <<EOF
#!/usr/bin/tclsh
# This profile includes all the rules for checking NEST
set rules {
  F001
  F002
  L001
  L002
  L003
  L005
  L006
  T001
  T002
  T004
  T005
  T006
  T007
  T010
  T011
  T012
  T013
  T017
  T018
  T019
}
EOF
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
  VERA_VERS=`vera++ --version`
  CPPCHECK_VERS=`cppcheck --version`
  CLANG_FORMAT_VERS=`clang-format --version`
  echo "MSGBLD0065: Following tools are in use:"
  echo "MSGBLD0065: VERA++       : $VERA_VERS"
  echo "MSGBLD0065: CPPCHECK     : $CPPCHECK_VERS"
  echo "MSGBLD0065: CLANG-FORMAT : $CLANG_FORMAT_VERS"
  
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

  printf '%s\n' "$file_names" | while IFS= read -r line
  do
    echo "MSGBLD0095: File changed: $line"
  done
  echo "MSGBLD0100: Retrieving changed files completed."

  format_error_files=""
  for f in $file_names; do
    if [ ! -f "$f" ]; then
      echo "MSGBLD0110: $f is not a file or does not exist anymore."
      continue
    fi
    echo "MSGBLD0120: Perform static code analysis for file $f."
    case $f in
      *.h | *.c | *.cc | *.hpp | *.cpp )
        f_base=$NEST_VPATH/reports/`basename $f`

        echo "\n"
        echo "MSGBLD0130: Run VERA++ for file: $f"
        vera++ --root ./vera_home --profile nest $f > ${f_base}_vera.txt 2>&1
        cat ${f_base}_vera.txt | while read line
        do
          echo "MSGBLD0135: VERA++ $line"
        done
        echo "MSGBLD0140: VERA++ for file $f completed."

        echo "MSGBLD0150: Run CPPCHECK for file: $f"
        cppcheck --enable=all --inconclusive --std=c++03 $f > ${f_base}_cppcheck.txt 2>&1
        cat ${f_base}_cppcheck.txt | while read line
        do
          echo "MSGBLD0155: CPPCHECK $line"
        done        
        echo "MSGBLD0160: CPPCHECK for file $f completed."

        echo "MSGBLD0170: Run CLANG-FORMAT for file: $f"
        # Create a clang-format formatted temporary file and perform a diff with its origin.
        clang-format $f > ${f_base}_formatted_$TRAVIS_COMMIT.txt
        diff $f ${f_base}_formatted_$TRAVIS_COMMIT.txt | tee ${f_base}_clang_format.txt
        cat ${f_base}_clang_format.txt | while read line
        do
          echo "MSGBLD0175: CLANG-FORMAT $line"
        done        
        echo "MSGBLD0180: CLANG-FORMAT for file $f completed."

        # Remove temporary files.
        rm ${f_base}_formatted_$TRAVIS_COMMIT.txt

        if [ -s ${f_base}_clang_format.txt ]; then
          format_error_files="$format_error_files $f"
        fi

        ;;
      *.py )
        echo "MSGBLD0190: Run PEP8 check for file: $f"
        # Ignore those PEP8 rules.
        PEP8_IGNORES="E121,E123,E126,E226,E24,E704"
        PEP8_IGNORES_EXAMPLES="${PEP8_IGNORES},E402"
        # Regular expression of directory patterns on which to apply PEP8_IGNORES_EXAMPLES.
        case $f in
          *examples* | *user_manual_scripts*)
            IGNORES=$PEP8_IGNORES_EXAMPLES
            ;;
          *)
            IGNORES=$PEP8_IGNORES
            ;;
        esac

        if ! pep8_result=`pep8 --ignore=$PEP8_IGNORES $f` ; then
          printf '%s\n' "$pep8_result" | while IFS= read -r line
          do
            echo "MSGBLD0195: PEP8 $line"
          done
          format_error_files="$format_error_files $f"
        fi
        echo "MSGBLD0200: PEP8 check for file $f completed."
        ;;
      *)
        echo "MSGBLD0210: File $f is not a C/CPP/PY file. Static code analysis and formatting check skipped."
        continue
    esac
  done

  if [ "x$format_error_files" != "x" ]; then
    printf '%s\n' "$format_error_files" | while IFS= read -r line
    do
      echo "MSGBLD0220: Formatting error in file: $line"
    done
  fi
fi   # static code analysis


cd "$NEST_VPATH"
cp ../examples/sli/nestrc.sli ~/.nestrc

echo "\n"
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
  ..
echo "MSGBLD0240: CMake configure completed."

echo "\n"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               B U I L D   N E S T                                           +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0250: Running Make."
make VERBOSE=1
echo "MSGBLD0260: Make completed."

echo "\n"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               I N S T A L L   N E S T                                       +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0270: Running make install."
make install
echo "MSGBLD0280: Make install completed."

echo "\n"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+               R U N   N E S T   T E S T S U I T E                           +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "MSGBLD0290: Running make installcheck."
make installcheck
echo "MSGBLD0300: Make installcheck completed."

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  echo "MSGBLD0310: This build was triggered by a pull request." >&2
  echo "MSGBLD0330: (WARNING) Build artifacts not uploaded to Amazon S3." >&2
fi

if [ "$TRAVIS_REPO_SLUG" != "nest/nest-simulator" ] ; then
  echo "MSGBLD0320: This build was from a forked repository and not from nest/nest-simulator." >&2
  echo "MSGBLD0330: (WARNING) Build artifacts not uploaded to Amazon S3." >&2
fi
