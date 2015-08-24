#!/bin/bash

# static code analysis
START_SHA=master
END_SHA=HEAD

NEST_SRC=.
CPPCHECK=cppcheck
VERA=vera++
VERA_HOME=/usr/local/lib/vera++

CLANG_FORMAT=clang-format
INCREMENTAL=false

ONLY_FILE=

#
# usage [exit_code bad_option]
#
usage ()
{
    if test $1 -ne 0 ; then
        echo "Unknown option: $2"
    fi

    cat <<EOF
Usage: check_code_style.sh [options ...]"

Setup of Tooling is explained here: 
    https://nest.github.io/nest-simulator/coding_guidelines_c++

Options:

    --help               Print program options and exit
    --incremental        Do analysis one file after another.
    --file=/path/to/file Perform the static analysis on this file only.
    --git-start=SHA      Enter the default SHA for git to start the diff
                         (default=master)
    --git-end=SHA        Enter the default SHA for git to end the diff
                         (default=HEAD)
    --nest-src=/path     The base directory for the NEST sources
                         (default=. assuming you execute check_code_style.sh 
                         from the base directory.)
    --cppcheck=exe       Enter the executable that is used for cppcheck.
                         (default=cppcheck)
    --clang-format=exe   Enter the executable that is used for clang-format.
                         (default=clang-format)
    --vera++=exe         Enter the executable that is used for vera++.
                         (default=vera++)
EOF

    exit $1
}

#
# bail_out message
#
bail_out ()
{
    echo "$1"
    exit 1
}

while test $# -gt 0 ; do
    case "$1" in
        --help)
            usage 0
            ;;
        --incremental)
            INCREMENTAL=true
            ;;
        --file=*)
            ONLY_FILE="$( echo "$1" | sed 's/^--file=//' )"
            if [ ! -f $ONLY_FILE ]; then
              bail_out "This is not a valid file: $ONLY_FILE"
            fi
            ;;
        --git-start=*)
            START_SHA="$( echo "$1" | sed 's/^--git-start=//' )"
            ;;
        --git-end=*)
            END_SHA="$( echo "$1" | sed 's/^--git-end=//' )"
            ;;
        --nest-src=*)
            NEST_SRC="$( echo "$1" | sed 's/^--nest-src=//' )"
            ;;
        --cppcheck=*)
            CPPCHECK="$( echo "$1" | sed 's/^--cppcheck=//' )"
            ;;
        --vera++=*)
            VERA="$( echo "$1" | sed 's/^--vera++=//' )"
            ;;
        --clang-format=*)
            CLANG_FORMAT="$( echo "$1" | sed 's/^--clang-format=//' )"
            ;;
        *)
            usage 1 "$1"
            ;;
    esac
    shift
done

#
# sed has different syntax for extended regular expressions
# on different operating systems:
# BSD: -E
# other: -r
#
EXTENDED_REGEX_PARAM=r
/bin/sh -c "echo 'hello' | sed -${EXTENDED_REGEX_PARAM} 's/[aeou]/_/g' "  >/dev/null 2>&1 || EXTENDED_REGEX_PARAM=E

echo "Executing check_code_style.sh"
echo "Tooling:"

# check vera++ is set up appropriately
$VERA $NEST_SRC/nest/main.cpp >/dev/null 2>&1 || usage 1 "Executable $VERA for vera++ is not working!"
$VERA --profile nest $NEST_SRC/nest/main.cpp >/dev/null 2>&1 ||  bail_out "No profile called nest for vera++ installed. See https://nest.github.io/nest-simulator/coding_guidelines_c++#vera-profile-nest"
echo "vera++ version: `vera++ --version`"

# check cppcheck 1.69 is installed correctly
$CPPCHECK --enable=all --inconclusive --std=c++03 $NEST_SRC/nest/main.cpp >/dev/null 2>&1 || usage 1 "Executable $CPPCHECK for cppcheck is not working!"
cppcheck_version=`$CPPCHECK --version | sed 's/^Cppcheck //'`
echo "cppcheck version: $cppcheck_version"
if [[ "x$cppcheck_version" != "x1.69" ]]; then
  bail_out "Require cppcheck version 1.69. Not $cppcheck_version ."
fi

# clang-format is installed correctly
$CLANG_FORMAT -style=$NEST_SRC/.clang-format $NEST_SRC/nest/main.cpp >/dev/null 2>&1 || usage 1 "Executable $CLANG_FORMAT for clang-format is not working!"
clang_format_version=`$CLANG_FORMAT --version | sed -${EXTENDED_REGEX_PARAM} 's/^.*([0-9]\.[0-9])\..*/\1/'`
echo "clang-format version: $clang_format_version"
if [[ "x$clang_format_version" != "x3.6" ]]; then
  bail_out "Require clang-format version 3.6. Not $clang_format_version ."
fi

# Extracting changed files between two commits
if [[ "x$ONLY_FILE" != "x" && -f $ONLY_FILE ]]; then
  file_names=$ONLY_FILE
else
  file_names=`git diff --name-only $START_SHA..$END_SHA`
fi
format_error_files=""

echo
echo "Performing static analysis on:"
for f in $file_names; do
  echo "  $f"
done
if $INCREMENTAL; then
  echo "Press enter to continue."
  read
else
  echo
fi

for f in $file_names; do
  if [ ! -f "$NEST_SRC/$f" ]; then
    echo "$f : Is not a file or does not exist anymore."
    continue
  fi
  # filter files
  case $f in
    *.h | *.c | *.cc | *.hpp | *.cpp )
      echo
      echo "Static analysis on file $f:"
      if $INCREMENTAL; then
        echo "Press enter to continue."
        read
      else
        echo
      fi

      echo " - clang-format for $f: (critical: if there are diffs, perform $CLANG_FORMAT -i $f )"
      # clang format creates tempory formatted file
      $CLANG_FORMAT $NEST_SRC/$f > $NEST_SRC/${f}_formatted.txt
      # compare the committed file and formatted file and 
      # writes the differences to a temp file
      diff $f $NEST_SRC/${f}_formatted.txt | tee $NEST_SRC/${f}_clang_format.txt

      if [ -s $NEST_SRC/${f}_clang_format.txt ]; then 
        # file exists and has size greater than zero
        format_error_files="$format_error_files $f"
      fi

      # remove temporary files
      rm $NEST_SRC/${f}_formatted.txt
      rm $NEST_SRC/${f}_clang_format.txt

      # Vera++ checks the specified list of rules given in the profile 
      # nest which is placed in the <vera++ root>/lib/vera++/profile
      echo " - vera++ for $f: (please consider fixing all warnings emitted here.)"
      $VERA --profile nest $NEST_SRC/$f

      # perform other static analysis
      echo " - cppcheck for $f: (suggestions for improvements)"
      $CPPCHECK --enable=all --inconclusive --std=c++03 $NEST_SRC/$f

      ;;
    *)
      echo "$f : not a C/CPP file. Do not do static analysis / formatting checking."
  esac
done

if [ "$format_error_files" != "" ]; then
  echo
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  echo "There are files with a formatting error:"
  for f in $format_error_files; do
    echo "  $f"
  done
  echo
  echo "Perform '$CLANG_FORMAT -i <filename>' on them, otherwise TravisCI will report failure!"
  echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  exit 42
fi
