#!/bin/bash

# check_code_style.sh
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

# This script performs a static code analysis and can be used to verify
# if a source file fulfills the NEST coding style guidelines.
# Run ./build_support/check_code_style.sh --help for more detailed information.

# Set default parameters.
unset FILE_TO_BE_CHECKED
GIT_START_SHA=master             # If 'file=' is not specified all changed files in the commit range
GIT_END_SHA=HEAD                 # '<master>..<HEAD>' are processed.

CPPCHECK=cppcheck                # CPPCHECK version 1.69 or later is required!
CLANG_FORMAT=clang-format        # CLANG-FORMAT version 13 is required!
PEP8=pycodestyle
PYCODESTYLE_IGNORES="E121,E123,E126,E226,E24,E704,W503,W504"

PERFORM_CPPCHECK=false           # Skip CPPCHECK analysis.
PERFORM_CLANG_FORMAT=true        # Perform CLANG-FORMAT analysis.
PERFORM_PEP8=true                # Perform PEP8 analysis.

INCREMENTAL=false                # Do not prompt the user before each file analysis.

# Exit script on 'Unknown option' condition.
# error_unknown_option "option"
error_unknown_option() {
  echo "[ERROR] Unknown option: $1"
  echo
  print_usage
  exit 1
}

# Exit script on 'No static code analysis specified.'
error_no_analysis() {
  echo "[ERROR] No static code analysis specified."
  echo
  print_usage
  exit 1
}

# Exit script on error.
# error_exit "error_message"
error_exit() {
  echo "[ERROR] $1"
  echo
  exit 1
}

# Print usage and exit script.
usage() {
  print_usage
  exit 0
}

print_usage() {
    cat <<EOF
Usage: ./build_support/check_code_style.sh [options ...]

This script processes C/C++ and Python source code files to verify compliance with the NEST
coding  style  guidelines.  The  checks are performed the same way as in the NEST CI
build and test environment. If no file is specified, a local 'git diff' is issued to obtain
the changed files in the commit range '<git-sha-start>..<git-sha-end>'. By default, this is
'master..head'.

The script expects to be run from the base directory of the NEST sources,
i.e. all executions should start like:
    ./build_support/check_code_style.sh ...

The setup of the tooling is explained here:
    https://nest-simulator.readthedocs.io/en/latest/contribute/coding_guidelines_cpp.html

Options:

    --help                           This help.

    --[i]ncremental                  Prompt user before each file analysis.

    --file=/path/to/file             Perform the analysis on this file.

    --git-start=Git_SHA_value        Hash value (Git SHA) from which Git starts the diff.
                                     Default: --git-start=master

    --git-end=Git_SHA_value          Hash value (Git SHA) at which Git ends the diff.
                                     Default: --git-start=HEAD

    --cppcheck=exe                   The name of the CPPCHECK executable.
                                     Default: --cppcheck=cppcheck
                                     Note: CPPCHECK version 1.69 or later is required.
                                           This corresponds to the version installed in
                                           the NEST CI build and test environment.

    --clang-format=exe               The name of the CLANG-FORMAT executable.
                                     Default: --clang-format=clang-format
                                     Note: CLANG-FORMAT version 13 is required.

    --pep8=exe                       The name of the PEP8 executable.
                                     Default: --pep8=pycodestyle

    --perform-cppcheck=on/off        Turn on/off CPPCHECK analysis.
                                     Default: --perform-cppcheck=off

    --perform-clang-format=on/off    Turn on/off CLANG-FORMAT analysis.
                                     Default: --perform-clang-format=on

    --perform-pep8=on/off            Turn on/off PEP8 analysis.
                                     Default: --perform-pep8=on
EOF
echo
}

echo
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo "+                       NEST STATIC CODE ANALYSIS TOOL                        +"
echo "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
echo

# Process parameters.
while test $# -gt 0 ; do
  case "$1" in
      --help)
          usage
          ;;
      --i)
          INCREMENTAL=true
          ;;
      --incremental)
          INCREMENTAL=true
          ;;
      --file=*)
          FILE_TO_BE_CHECKED="$( echo "$1" | sed 's/^--file=//' )"
          if [ ! -f $FILE_TO_BE_CHECKED ]; then
            error_exit "The specified file does not exist. File: $FILE_TO_BE_CHECKED"
          fi
          ;;
      --git-start=*)
          GIT_START_SHA="$( echo "$1" | sed 's/^--git-start=//' )"
          ;;
      --git-end=*)
          GIT_END_SHA="$( echo "$1" | sed 's/^--git-end=//' )"
          ;;
      --cppcheck=*)
          CPPCHECK="$( echo "$1" | sed 's/^--cppcheck=//' )"
          ;;
      --clang-format=*)
          CLANG_FORMAT="$( echo "$1" | sed 's/^--clang-format=//' )"
          ;;
      --pep8=*)
          PEP8="$( echo "$1" | sed 's/^--pep8=//' )"
          ;;
      --perform-cppcheck=*)
          value="$( echo "$1" | sed 's/^--perform-cppcheck=//' )"
          case "$value" in
            on)
                PERFORM_CPPCHECK=true
            ;;
            off)
                PERFORM_CPPCHECK=false
            ;;
            *)
                error_unknown_option "$1"
            ;;
          esac
          ;;
      --perform-clang-format=*)
          value="$( echo "$1" | sed 's/^--perform-clang-format=//' )"
          case "$value" in
            on)
                PERFORM_CLANG_FORMAT=true
            ;;
            off)
                PERFORM_CLANG_FORMAT=false
            ;;
            *)
                error_unknown_option "$1"
            ;;
          esac
          ;;
      --perform-pep8=*)
          value="$( echo "$1" | sed 's/^--perform-pep8=//' )"
          case "$value" in
            on)
                PERFORM_PEP8=true
            ;;
            off)
                PERFORM_PEP8=false
            ;;
            *)
                error_unknown_option "$1"
            ;;
          esac
          ;;
      *)
          error_unknown_option "$1"
          ;;
  esac
  shift
done

if ! $PERFORM_CPPCHECK && ! $PERFORM_CLANG_FORMAT && ! $PERFORM_PEP8; then
  error_no_analysis
fi

# Evaluate sed extended regex parameter.
# The sed syntax for extended regular expressions differs for the operating systems: BSD: -E  other: -r
EXTENDED_REGEX_PARAM=r
/bin/sh -c "echo 'hello' | sed -${EXTENDED_REGEX_PARAM} 's/[aeou]/_/g' "  >/dev/null 2>&1 || EXTENDED_REGEX_PARAM=E

# Verify the CPPCHECK installation. CPPCHECK version 1.69 or later is required.
# Previous versions of CPPCHECK halted on sli/tokenutils.cc (see https://github.com/nest/nest-simulator/pull/79)
if $PERFORM_CPPCHECK; then
  $CPPCHECK --enable=all --inconclusive --std=c++03 ./nest/main.cpp >/dev/null 2>&1 || error_exit "Failed to verify the CPPCHECK installation. Executable: $CPPCHECK"
  cppcheck_version=`$CPPCHECK --version | sed 's/^Cppcheck //'`
  IFS=\. read -a cppcheck_version_a <<< "$cppcheck_version"
  if [[ ${cppcheck_version_a[0]} -lt 1 ]]; then
    error_exit "Failed to verify the CPPCHECK installation. Version 1.69 or later is requires. The executable '$CPPCHECK' is of version $cppcheck_version."
  elif [[ ${cppcheck_version_a[0]} -eq 1 && ${cppcheck_version_a[1]} -lt 69 ]]; then
    error_exit "Failed to verify the CPPCHECK installation. Version 1.69 or later is requires. The executable '$CPPCHECK' is of version $cppcheck_version."
  fi
fi

# Verify the CLANG-FORMAT installation. CLANG-FORMAT version 13 is required.
if $PERFORM_CLANG_FORMAT; then
  $CLANG_FORMAT -style=file ./nest/main.cpp >/dev/null 2>&1 || error_exit "Failed to verify the CLANG-FORMAT installation. Executable: $CLANG_FORMAT"
  clang_format_version=`$CLANG_FORMAT --version | sed -${EXTENDED_REGEX_PARAM} 's/^[^0-9]*([0-9]+\.[0-9]+)\..*/\1/'`
  if [[ "x$clang_format_version" != "x13.0" ]]; then
    error_exit "Failed to verify the CLANG-FORMAT installation. Version 13.0 is required. The executable '$CLANG_FORMAT' is version $clang_format_version."
  fi
fi

# Verify the pycodestyle installation.
if $PERFORM_PEP8; then
  $PEP8 --ignore=$PYCODESTYLE_IGNORES ./build_support/parse_build_log.py || error_exit "Failed to verify the pycodestyle installation. Executable: $PEP8"
fi

# Extracting changed files between two commits.
if [[ "x$FILE_TO_BE_CHECKED" != "x" && -f $FILE_TO_BE_CHECKED ]]; then
  file_names=$FILE_TO_BE_CHECKED
else
  file_names=`git diff --name-only $GIT_START_SHA..$GIT_END_SHA`
fi

if [ -z "$file_names" ]; then
  echo
  echo "There are no files to check."
  exit 0
fi

if [ ! -x ./build_support/static_code_analysis.sh ]; then
  sudo chmod +x ./build_support/static_code_analysis.sh
fi


RUNS_ON_CI=false

unset NEST_VPATH               # These command line arguments are placeholders and not required here.
IGNORE_MSG_CPPCHECK=false      # the CI build environment.
IGNORE_MSG_CLANG_FORMAT=false
IGNORE_MSG_PYCODESTYLE=false

./build_support/static_code_analysis.sh "$RUNS_ON_CI" "$INCREMENTAL" "$file_names" "$NEST_VPATH" \
"$CPPCHECK" "$CLANG_FORMAT" "$PEP8" \
"$PERFORM_CPPCHECK" "$PERFORM_CLANG_FORMAT" "$PERFORM_PEP8" \
"$IGNORE_MSG_CPPCHECK" "$IGNORE_MSG_CLANG_FORMAT" "$IGNORE_MSG_PYCODESTYLE" \
"$PYCODESTYLE_IGNORES"
if [ $? -gt 0 ]; then
    exit $?
fi
