#!/bin/bash

# static code analysis
START_SHA=master
END_SHA=HEAD

CPPCHECK=cppcheck
VERA=vera++

CLANG_FORMAT=clang-format-3.6
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
Usage: ./extras/check_code_style.sh [options ...]"

This script checks our coding style guidelines. The checks are
performed in the same way as in our TravisCI setup. First it looks
for changed files in the repository in the commit range <git-start>..<git-end>
(by default this is master..HEAD). Only the changed files are checked.
You can specify a different commit range, or define the file to be checked,
as an argument.

The script expects to be run from the base directory of the
NEST sources, i.e. all executions should start like:
    ./extras/check_code_style.sh ...

Setup of the tooling is explained here: 
    https://nest.github.io/nest-simulator/coding_guidelines_c++

Options:

    --help               Print program options and exit.
    --incremental        Do analysis on one file after another.
    --file=/path/to/file Perform the static analysis on this file only.
    --git-start=SHA      Enter the SHA from which git starts the diff.
                         (default=master)
    --git-end=SHA        Enter the SHA from which git ends the diff.
                         (default=HEAD)
    --cppcheck=exe       Enter the executable that is used for cppcheck. Due to
                         a bug in previous versions we require at least version
                         1.69, which is also used in our TravisCI setup.
                         (default=cppcheck)
    --clang-format=exe   Enter the executable that is used for clang-format.
                         Due to formatting differences in version 3.6 and 3.7
                         we require version 3.6, which is also used in our
                         TravisCI setup. (default=clang-format-3.6)
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
$VERA ./nest/main.cpp >/dev/null 2>&1 || usage 1 "Executable $VERA for vera++ is not working!"
$VERA --profile nest ./nest/main.cpp >/dev/null 2>&1 ||  bail_out "No profile called nest for vera++ installed. See https://nest.github.io/nest-simulator/coding_guidelines_c++#vera-profile-nest"
echo "vera++ version: `vera++ --version`"

# check cppcheck 1.69 is installed correctly
# Previous versions of cppcheck halted on sli/tokenutils.cc (see https://github.com/nest/nest-simulator/pull/79)
$CPPCHECK --enable=all --inconclusive --std=c++03 ./nest/main.cpp >/dev/null 2>&1 || usage 1 "Executable $CPPCHECK for cppcheck is not working!"
cppcheck_version=`$CPPCHECK --version | sed 's/^Cppcheck //'`
echo "cppcheck version: $cppcheck_version"
IFS=\. read -a cppcheck_version_a <<< "$cppcheck_version"
if [[ ${cppcheck_version_a[0]} -lt 1 ]]; then
  bail_out "Require cppcheck version 1.69 or later. Not $cppcheck_version ."
elif [[ ${cppcheck_version_a[0]} -eq 1 && ${cppcheck_version_a[1]} -lt 69 ]]; then
  bail_out "Require cppcheck version 1.69 or later. Not $cppcheck_version ."
fi

# clang-format is installed correctly
# clang-format version 3.5 and before do not understand all configuration
# options we use. Version 3.7 has a different formatting behaviour. Since the
# sources are formatted with 3.6, we require clang-format 3.6 for the checking.
$CLANG_FORMAT -style=./.clang-format ./nest/main.cpp >/dev/null 2>&1 || usage 1 "Executable $CLANG_FORMAT for clang-format is not working!"
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

if [ -z "$file_names" ]; then
  echo
  echo "Nothing to check."
  exit 0
fi

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
  if [ ! -f $f ]; then
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
      $CLANG_FORMAT $f > ${f}_formatted.txt
      # compare the committed file and formatted file and 
      # writes the differences to a temp file
      diff $f ${f}_formatted.txt | tee ${f}_clang_format.txt

      if [ -s ${f}_clang_format.txt ]; then 
        # file exists and has size greater than zero
        format_error_files="$format_error_files $f"
      fi

      # remove temporary files
      rm ${f}_formatted.txt
      rm ${f}_clang_format.txt

      # Vera++ checks the specified list of rules given in the profile 
      # nest which is placed in the <vera++ root>/lib/vera++/profile
      echo " - vera++ for $f: (please consider fixing all warnings emitted here.)"
      $VERA --profile nest $f

      # perform other static analysis
      echo " - cppcheck for $f: (suggestions for improvements)"
      $CPPCHECK --enable=all --inconclusive --std=c++03 $f

      ;;
    *)
      echo "$f : not a C/CPP file. Skipping ..."
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
