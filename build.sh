#!/bin/sh

set -e
set -x

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

# static code analysis
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


if [ ! -f "$HOME/.cache/bin/cppcheck" ]; then
  # initialize and build cppcheck 1.69
  git clone https://github.com/danmar/cppcheck.git
  # go into source directory of cppcheck
  cd cppcheck
  # set git to 1.69 version
  git checkout tags/1.69
  # build cppcheck => now there is an executable ./cppcheck
  mkdir -p install
  make PREFIX=$HOME/.cache CFGDIR=$HOME/.cache/cfg HAVE_RULES=yes install

  cd ..
  
  wget http://llvm.org/releases/3.6.2/clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz
  tar xvf clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04.tar.xz
  
  # copy, not move, since .cache may contain other files in subdirs already
  cp -R clang+llvm-3.6.2-x86_64-linux-gnu-ubuntu-14.04/* $HOME/.cache
  
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
echo "Extract changed files..."
if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  file_names=`curl "https://api.github.com/repos/$TRAVIS_REPO_SLUG/pulls/$TRAVIS_PULL_REQUEST/files" | jq '.[] | .filename' | tr '\n' ' ' | tr '"' ' '`
else
  # extract filenames via git => has some problems with history rewrites
  # see https://github.com/travis-ci/travis-ci/issues/2668
  file_names=`(git diff --name-only $TRAVIS_COMMIT_RANGE || echo "") | tr '\n' ' '`
fi
format_error_files=""

# Ignore those PEP8 rules
PEP8_IGNORES="E121,E123,E126,E226,E24,E704"

# In example dirs, also ignore incorrectly placed imports
PEP8_IGNORES_EXAMPLES="${PEP8_IGNORES},E402"

# regular expression of directory patterns on which to apply
# PEP8_IGNORES_EXAMPLES
EXAMPLE_DIRS='examples|user_manual_scripts'

for f in $file_names; do
  if [ ! -f "$f" ]; then
    echo "$f : Is not a file or does not exist anymore."
    continue
  fi
  # filter files
  case $f in
    *.h | *.c | *.cc | *.hpp | *.cpp )
      echo "Static analysis on file $f:"
      f_base=$NEST_VPATH/reports/`basename $f`
      # Vera++ checks the specified list of rules given in the profile
      # nest which is placed in the <vera++ root>/lib/vera++/profile
      vera++ --root ./vera_home --profile nest $f > ${f_base}_vera.txt 2>&1
      echo "\n - vera++ for $f:"
      cat ${f_base}_vera.txt

      cppcheck --enable=all --inconclusive --std=c++03 $f > ${f_base}_cppcheck.txt 2>&1
      echo "\n - cppcheck for $f:"
      cat ${f_base}_cppcheck.txt

      # clang format creates tempory formatted file
      clang-format $f > ${f_base}_formatted_$TRAVIS_COMMIT.txt
      # compare the committed file and formatted file and
      # writes the differences to a temp file
      echo "\n - clang-format for $f:"
      diff $f ${f_base}_formatted_$TRAVIS_COMMIT.txt | tee ${f_base}_clang_format.txt
      cat ${f_base}_clang_format.txt

      # remove temporary files
      rm ${f_base}_formatted_$TRAVIS_COMMIT.txt

      if [ -s ${f_base}_clang_format.txt ]; then
        # file exists and has size greater than zero
        format_error_files="$format_error_files $f"
      fi

      ;;
    *.py )
      echo "Check PEP8 on file $f:"

      if [[ $f =~ $EXAMPLE_DIRS ]]; then
        IGNORES=$PEP8_IGNORES_EXAMPLES
      else
        IGNORES=$PEP8_IGNORES
      fi

      if ! pep8_result=`pep8 --first --ignore=$PEP8_IGNORES $f` ; then
        echo "$pep8_result"

        format_error_files="$format_error_files $f"
      fi
      ;;
    *)
      echo "$f : not a C/CPP/PY file. Do not do static analysis / formatting checking."
      continue
  esac
done


cd "$NEST_VPATH"

cmake \
  -DCMAKE_INSTALL_PREFIX="$NEST_RESULT" \
  -Dwith-optimize=ON \
  -Dwith-warning=ON \
  $CONFIGURE_MPI \
  $CONFIGURE_PYTHON \
  $CONFIGURE_GSL \
  ..

make VERBOSE=1
make install
make installcheck

if [ "x$format_error_files" != "x" ]; then
  echo "There are files with a formatting error: $format_error_files ."
  exit 42
fi

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  echo "WARNING: Not uploading results as this is a pull request" >&2
  exit 0
fi
