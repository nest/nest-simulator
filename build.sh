#!/bin/sh

# Enables checking of all commands. If a command exits with an error and the
# caller does not check such error, the script aborts immediately.
set -e

# uncomment this command, if you debug the build; it outputs subsequent command
# before they are executed
# set -x

# This script is used during the continuous integration tests with TravisCI. After
# its execution the Python script `extras/parse_travis_log.py` parses the output
# and produces a summary of the static analysis, and of building and testing NEST.
# Changes to the output might break the parsing. Please fix the script
# `extras/parse_travis_log.py` in this case as well.


mkdir -p $HOME/.matplotlib
cat > $HOME/.matplotlib/matplotlibrc <<EOF
    # ZYV
    backend : svg
EOF

if [ "$xMPI" = "1" ] ; then

cat > $HOME/.nestrc <<EOF
    % ZYV: NEST MPI configuration
    /mpirun
    [/integertype /stringtype]
    [/numproc     /slifile]
    {
     () [
      (mpirun -np ) numproc cvs ( ) statusdict/prefix :: (/bin/nest )  slifile
     ] {join} Fold
    } Function def
EOF

    CONFIGURE_MPI="-Dwith-mpi=ON"

else
    CONFIGURE_MPI="-Dwith-mpi=OFF"
fi

if [ "$xPYTHON" = "1" ] ; then
    CONFIGURE_PYTHON="-Dwith-python=ON"
else
    CONFIGURE_PYTHON="-Dwith-python=OFF"
fi

if [ "$xGSL" = "1" ] ; then
    CONFIGURE_GSL="-Dwith-gsl=ON"
else
    CONFIGURE_GSL="-Dwith-gsl=OFF"
fi

NEST_VPATH=build
NEST_RESULT=result

mkdir "$NEST_VPATH" "$NEST_RESULT"
mkdir "$NEST_VPATH/reports"

NEST_RESULT=$(readlink -f $NEST_RESULT)

if [ "$xSTATIC_ANALYSIS" = "1" ] ; then

  # static code analysis
  echo "======= VERA++ init start ======="
  # initialize vera++
  mkdir -p vera_home

  # on ubuntu vera++ is installed to /usr/
  # copy all scripts/rules/ ... into ./vera_home
  cp -r /usr/lib/vera++/* ./vera_home
  # create the nest-profile for vera++
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
  echo "======= VERA++ init end ======="

  if [ ! -f "$HOME/.cache/bin/cppcheck" ]; then
    echo "======= CPPCHECK init start ======="
    # initialize and build cppcheck 1.69
    git clone https://github.com/danmar/cppcheck.git
    # go into source directory of cppcheck
    cd cppcheck
    # set git to 1.69 version
    git checkout tags/1.69
    # build cppcheck => now there is an executable ./cppcheck
    mkdir -p install
    make PREFIX=$HOME/.cache CFGDIR=$HOME/.cache/cfg HAVE_RULES=yes install
    echo "======= CPPCHECK init end ======="

    cd ..
    echo "======= CLANG-FORMAT init start ======="
    wget http://llvm.org/releases/3.6.2/clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz
    tar xf clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz

    # copy, not move, since .cache may contain other files in subdirs already
    cp -R clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04/* $HOME/.cache
    echo "======= CLANG-FORMAT init end ======="
    # remove directories, otherwise copyright-header check complains
    rm -rf ./cppcheck
    rm -rf ./clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04

  fi

  # Prepend cache to PATH so we find stuff we have installed ourselves first
  export PATH=$HOME/.cache/bin:$PATH

  vera++ --version
  cppcheck --version
  clang-format --version

  # Extracting changed files in PR / push
  echo "======= Extract changed files start ======="
  if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    file_names=`curl "https://api.github.com/repos/$TRAVIS_REPO_SLUG/pulls/$TRAVIS_PULL_REQUEST/files" | jq '.[] | .filename' | tr '\n' ' ' | tr '"' ' '`
  else
    # extract filenames via git => has some problems with history rewrites
    # see https://github.com/travis-ci/travis-ci/issues/2668
    file_names=`(git diff --name-only $TRAVIS_COMMIT_RANGE || echo "") | tr '\n' ' '`
  fi
  format_error_files=""
  echo "file_names=$file_names"
  echo "======= Extract changed files end ======="

  for f in $file_names; do
    if [ ! -f "$f" ]; then
      echo "$f : Is not a file or does not exist anymore."
      continue
    fi
    # filter files
    echo "======= Static analysis on file $f ======="
    case $f in
      *.h | *.c | *.cc | *.hpp | *.cpp )
        f_base=$NEST_VPATH/reports/`basename $f`
        # Vera++ checks the specified list of rules given in the profile
        # nest which is placed in the <vera++ root>/lib/vera++/profile

        echo "\n======= - vera++ for $f ======="
        vera++ --root ./vera_home --profile nest $f > ${f_base}_vera.txt 2>&1
        cat ${f_base}_vera.txt
        echo "======= - vera++ end ======="

        echo "\n======= - cppcheck for $f ======="
        cppcheck --enable=all --inconclusive --std=c++03 $f > ${f_base}_cppcheck.txt 2>&1
        cat ${f_base}_cppcheck.txt
        echo "======= - cppcheck end ======="

        echo "\n======= - clang-format for $f ======="
        # clang format creates tempory formatted file
        clang-format $f > ${f_base}_formatted_$TRAVIS_COMMIT.txt
        # compare the committed file and formatted file and
        # writes the differences to a temp file
        diff $f ${f_base}_formatted_$TRAVIS_COMMIT.txt | tee ${f_base}_clang_format.txt
        cat ${f_base}_clang_format.txt
        echo "======= - clang-format end ======="

        # remove temporary files
        rm ${f_base}_formatted_$TRAVIS_COMMIT.txt

        if [ -s ${f_base}_clang_format.txt ]; then
          # file exists and has size greater than zero
          format_error_files="$format_error_files $f"
        fi

        ;;
      *.py )
        echo "======= Check PEP8 for $f ======="

        # Ignore those PEP8 rules
        PEP8_IGNORES="E121,E123,E126,E226,E24,E704"

        # In example dirs, also ignore incorrectly placed imports
        PEP8_IGNORES_EXAMPLES="${PEP8_IGNORES},E402"
        # regular expression of directory patterns on which to apply
        # PEP8_IGNORES_EXAMPLES
        case $f in
          *examples* | *user_manual_scripts*)
            IGNORES=$PEP8_IGNORES_EXAMPLES
            ;;
          *)
            IGNORES=$PEP8_IGNORES
            ;;
        esac

        if ! pep8_result=`pep8 --first --ignore=$PEP8_IGNORES $f` ; then
          echo "$pep8_result"

          format_error_files="$format_error_files $f"
        fi
        echo "======= Check PEP8 end ======="
        ;;
      *)
        echo "$f : not a C/CPP/PY file. Do not do static analysis / formatting checking."
        continue
    esac
    echo "======= Static analysis end ======="
  done

  if [ "x$format_error_files" != "x" ]; then
    echo "There are files with a formatting error: $format_error_files ."
  fi
fi # if [ "$xSTATIC_ANALYSIS" = "1" ] ; then

cd "$NEST_VPATH"

echo "======= Configure NEST start ======="
cmake \
  -DCMAKE_INSTALL_PREFIX="$NEST_RESULT" \
  -Dwith-optimize=ON \
  -Dwith-warning=ON \
  $CONFIGURE_MPI \
  $CONFIGURE_PYTHON \
  $CONFIGURE_GSL \
  ..
echo "======= Configure NEST end ======="

echo "======= Make NEST start ======="
make VERBOSE=1
echo "======= Make NEST end ======="

echo "======= Install NEST start ======="
make install
echo "======= Install NEST end ======="

echo "======= Test NEST start ======="
make installcheck
echo "======= Test NEST end ======="

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  echo "WARNING: Not uploading results as this is a pull request" >&2
fi

if [ "$TRAVIS_REPO_SLUG" != "nest/nest-simulator" ] ; then
  echo "WARNING: Not uploading results as this is from a forked repository" >&2
fi
