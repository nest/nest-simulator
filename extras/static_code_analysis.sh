#!/bin/bash

# static_code_analysis.sh
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
# It performs the static code analysis and is invoked by 'travis_build.sh'.
# The script is also executed when running 'check_code_style.sh' for
# a local static code analysis.
#
# NOTE: This shell script is tightly coupled to Python script
#       'extras/parse_travis_log.py'.
#       Any changes to message numbers (MSGBLDnnnn) have effects on
#       the build/test-log parsing process.
#

# Command line parameters.
RUNS_ON_TRAVIS=${1}           # true or false, indicating whether the script is executed on Travis CI or runs local.
INCREMENTAL=${2}              # true or false, user needs to confirm befor checking a source file.
FILE_NAMES=${3}               # The list of files or a single file to be checked.
NEST_VPATH=${4}               # The high level NEST build path.
VERA=${5}                     # Name of the VERA++ executable.
CPPCHECK=${6}                 # Name of the CPPCHECK executable.
CLANG_FORMAT=${7}             # Name of the CLANG-FORMAT executable.
PEP8=${8}                     # Name of the PEP8 executable.
PERFORM_VERA=${9}             # true or false, indicating whether VERA++ analysis is performed or not.
PERFORM_CPPCHECK=${10}        # true or false, indicating whether CPPCHECK analysis is performed or not.
PERFORM_CLANG_FORMAT=${11}    # true or false, indicating whether CLANG-FORMAT analysis is performed or not.
PERFORM_PEP8=${12}            # true or false, indicating whether PEP8 analysis is performed or not.
IGNORE_MSG_VERA=${13}         # true or false, indicating whether VERA++ messages should accout for the build result.
IGNORE_MSG_CPPCHECK=${14}     # true or false, indicating whether CPPCHECK messages should accout for the build result.
IGNORE_MSG_CLANG_FORMAT=${15} # true or false, indicating whether CLANG-FORMAT messages should accout for the build result.
IGNORE_MSG_PEP8=${16}         # true or false, indicating whether PEP8 messages should accout for the build result.

# PEP8 rules to ignore.
PEP8_IGNORES="E121,E123,E126,E226,E24,E704"
PEP8_IGNORES_EXAMPLES="${PEP8_IGNORES},E402"
PEP8_IGNORES_TOPO_MANUAL="${PEP8_IGNORES_EXAMPLES},E265"

# PEP8 rules.
PEP8_MAX_LINE_LENGTH=120

# Constants
typeset -i MAX_CPPCHECK_MSG_COUNT=10

# Drop files that should not be checked (space-separated list).
FILES_TO_IGNORE="libnestutil/compose.hpp librandom/knuthlfg.h librandom/knuthlfg.cpp"

# Print a message.
# The format of the message depends on whether the script is executed on Travis CI or runs local.
# print_msg "string1" "string2"
print_msg() {
  if $RUNS_ON_TRAVIS; then
    echo "$1$2"
  else
    echo "$2"
  fi
}

# Print version information.
print_msg "MSGBLD0105: " "Following tools are in use:"
print_msg "MSGBLD0105: " "---------------------------"
if $PERFORM_VERA; then
  VERA_VERS=`$VERA --version`
  print_msg "MSGBLD0105: " "VERA++       : $VERA_VERS"
fi
if $PERFORM_CPPCHECK; then
  CPPCHECK_VERS=`$CPPCHECK --version | sed 's/^Cppcheck //'`
  print_msg "MSGBLD0105: " "CPPCHECK     : $CPPCHECK_VERS"
fi
if $PERFORM_CLANG_FORMAT; then
  CLANG_FORMAT_VERS=`$CLANG_FORMAT --version`
  print_msg "MSGBLD0105: " "CLANG-FORMAT : $CLANG_FORMAT_VERS"
fi
if $PERFORM_PEP8; then
  PEP8_VERS=`$PEP8 --version`
  print_msg "MSGBLD0105: " "PEP8         : $PEP8_VERS"
fi
print_msg "" ""

# The following messages report on the command line arguments IGNORE_MSG_xxx which indicate whether
# static code analysis error messages will cause the Travis CI build to fail or are ignored.
if $RUNS_ON_TRAVIS; then
  if $IGNORE_MSG_VERA; then
    print_msg "MSGBLD1010: " "IGNORE_MSG_VERA is set. VERA++ messages will not cause the build to fail."
  fi
  if $IGNORE_MSG_CPPCHECK; then
    print_msg "MSGBLD1020: " "IGNORE_MSG_CPPCHECK is set. CPPCHECK messages will not cause the build to fail."
  fi
  if $IGNORE_MSG_CLANG_FORMAT; then
    print_msg "MSGBLD1030: " "IGNORE_MSG_CLANG_FORMAT is set. CLANG_FORMAT messages will not cause the build to fail."
  fi
  if $RUNS_ON_TRAVIS && $IGNORE_MSG_PEP8; then
    print_msg "MSGBLD1040: " "IGNORE_MSG_PEP8 is set. PEP8 messages will not cause the build to fail."
  fi
fi


export NEST_SOURCE=$PWD
print_msg "MSGBLD1050: " "Running check for copyright headers"
copyright_check_errors=`python3 extras/check_copyright_headers.py`
print_msg "MSGBLD1060: " "Running sanity check for Name definition and usage"
unused_names_errors=`python3 extras/check_unused_names.py`
print_msg "MSGBLD1070: " "Running check for forbidden type usage"
forbidden_types_errors=`bash extras/check_forbidden_types.sh`


# Perfom static code analysis.
c_files_with_errors=""
python_files_with_errors=""
for f in $FILE_NAMES; do

  if [[ $FILES_TO_IGNORE =~ .*$f.* ]]; then
    print_msg "MSGBLD0110: " "$f is explicitly ignored."
    continue
  fi
  if [ ! -f "$f" ]; then
    print_msg "MSGBLD0110: " "$f is not a file or does not exist anymore."
    continue
  fi
  if ! $RUNS_ON_TRAVIS && $INCREMENTAL; then
    print_msg "" ""
    print_msg "" "Press [Enter] to continue.  (Static code analysis for file $f.)"
    read continue
  fi
  if $RUNS_ON_TRAVIS; then
    print_msg "MSGBLD0120: " "Perform static code analysis for file $f."
  fi

  case $f in
    *.h | *.c | *.cc | *.hpp | *.cpp )
      vera_failed=false
      cppcheck_failed=false
      clang_format_failed=false

      if $RUNS_ON_TRAVIS; then
        f_base=$NEST_VPATH/reports/`basename $f`
      else
        f_base=`basename $f`
      fi

      # VERA++
      if $PERFORM_VERA; then
        print_msg "MSGBLD0130: " "Running VERA++ .....: $f"
        $VERA --profile nest $f > ${f_base}_vera.txt 2>&1
        if [ -s "${f_base}_vera.txt" ]; then
          vera_failed=true
          cat ${f_base}_vera.txt | while read line
          do
            print_msg "MSGBLD0135: " "[VERA] $line"
          done
        fi
        rm ${f_base}_vera.txt
        if $RUNS_ON_TRAVIS; then
          print_msg "MSGBLD0140: " "VERA++ for file $f completed."
        fi
      fi

      # CPPCHECK
      if $PERFORM_CPPCHECK; then
        print_msg "MSGBLD0150: " "Running CPPCHECK ...: $f"
        $CPPCHECK --enable=all --language=c++ --std=c++11 --suppress=missingIncludeSystem $f > ${f_base}_cppcheck.txt 2>&1
        # Remove the header, the first line.
        tail -n +2 "${f_base}_cppcheck.txt" > "${f_base}_cppcheck.tmp" && mv "${f_base}_cppcheck.tmp" "${f_base}_cppcheck.txt"
        if [ -s "${f_base}_cppcheck.txt" ]; then
          cppcheck_failed=true
          typeset -i msg_count=0
          cat ${f_base}_cppcheck.txt | while read line
          do
            print_msg "MSGBLD0155: " "[CPPC] $line"
            if $RUNS_ON_TRAVIS; then
              msg_count+=1
              if [ ${msg_count} -ge ${MAX_CPPCHECK_MSG_COUNT} ]; then
                print_msg "MSGBLD0156: " "[CPPC] MAX_CPPCHECK_MSG_COUNT (${MAX_CPPCHECK_MSG_COUNT}) reached for file: $f"
                break
              fi
            fi
          done
        fi
        rm ${f_base}_cppcheck.txt
        if $RUNS_ON_TRAVIS; then
          print_msg "MSGBLD0160: " "CPPCHECK for file $f completed."
        fi
      fi

      # CLANG-FORMAT
      if $PERFORM_CLANG_FORMAT; then
        print_msg "MSGBLD0170: " "Running CLANG-FORMAT: $f"
        # Create a clang-format formatted temporary file and perform a diff with its origin.
        file_formatted="${f_base}_formatted.txt"
        file_diff="${f_base}_diff.txt"
        $CLANG_FORMAT $f > $file_formatted
        diff $f $file_formatted > $file_diff 2>&1
        if [ -s "$file_diff" ]; then
          clang_format_failed=true
          cat $file_diff | while read line
          do
            print_msg "MSGBLD0175: " "[DIFF] $line"
          done
        fi
        rm $file_formatted
        rm $file_diff
        if $RUNS_ON_TRAVIS; then
          print_msg "MSGBLD0180: " "CLANG-FORMAT for file $f completed."
        fi
      fi

      # Add the file to the list of files with format errors.
      if (! $IGNORE_MSG_VERA && $vera_failed) || (! $IGNORE_MSG_CPPCHECK && $cppcheck_failed) || (! $IGNORE_MSG_CLANG_FORMAT && $clang_format_failed); then
        c_files_with_errors="$c_files_with_errors $f"
      fi
      ;;

    *.py )
      # PEP8
      if $PERFORM_PEP8; then
        print_msg "MSGBLD0190: " "Running PEP8 .......: $f"
        case $f in
          *user_manual_scripts*)
            IGNORES=$PEP8_IGNORES_TOPO_MANUAL
            ;;
          *examples*)
            IGNORES=$PEP8_IGNORES_EXAMPLES
            ;;
          *)
            IGNORES=$PEP8_IGNORES
            ;;
        esac
        if ! pep8_result=`$PEP8 --max-line-length=$PEP8_MAX_LINE_LENGTH --ignore=$IGNORES $f` ; then
          printf '%s\n' "$pep8_result" | while IFS= read -r line
          do
            print_msg "MSGBLD0195: " "[PEP8] $line"
          done
          # Add the file to the list of files with format errors.
          python_files_with_errors="$python_files_with_errors $f"
        fi
        if $RUNS_ON_TRAVIS; then
          print_msg "MSGBLD0200: " "PEP8 check for file $f completed."
        fi
      fi
      ;;

    *)
      print_msg "MSGBLD0210: " "Skipping ...........: $f  (not a C/C++/Python file)"
      continue
  esac
done

nlines_copyright_check=`echo -e $copyright_check_errors | sed -e 's/^ *//' | wc -l`
if [ $nlines_copyright_check \> 1 ] || \
   [ "x$unused_names_errors" != "x" ] || \
   [ -n "$forbidden_types_errors" ] || \
   [ "x$c_files_with_errors" != "x" ] || \
   [ "x$python_files_with_errors" != "x" ]; then

  print_msg "" ""
  print_msg "MSGBLD0220: " "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
  print_msg "MSGBLD0220: " "+                 STATIC CODE ANALYSIS DETECTED PROBLEMS !                    +"
  print_msg "MSGBLD0220: " "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
  print_msg "" ""
  if [ "x$c_files_with_errors" != "x" ]; then
    print_msg "MSGBLD0220: " "C/C++ files with formatting errors:"
    for f in $c_files_with_errors; do
      print_msg "MSGBLD0220: " "... $f"
    done
    if ! $RUNS_ON_TRAVIS; then
      print_msg "" "Run $CLANG_FORMAT -i <filename> for autocorrection."
      print_msg "" ""
    fi
  fi

  if [ $nlines_copyright_check \> 1 ]; then
      print_msg "MSGBLD0220: " "Files with erroneous copyright headers:"
      echo -e $copyright_check_errors | sed -e 's/^ *//'
      print_msg "" ""
  fi

  if [ "x$unused_names_errors" != "x" ]; then
      print_msg "MSGBLD0220: " "Files with unused/ill-defined Name objects:"
      echo -e $unused_names_errors | sed -e 's/^ *//'
      print_msg "" ""
  fi

  if [ -n "$forbidden_types_errors" ]; then
      print_msg "MSGBLD0220: " "Files with forbidden types (hint: use types without _t suffix):"
      echo -e $forbidden_types_errors | sed -e 's/^ *//'
      print_msg "" ""
  fi

  if [ "x$python_files_with_errors" != "x" ]; then
    print_msg "MSGBLD0220: " "Python files with formatting errors:"
    for f in $python_files_with_errors; do
      print_msg "MSGBLD0220: " "... $f"
    done
    if ! $RUNS_ON_TRAVIS; then
      print_msg "" "Run pep8ify -w <filename> for autocorrection."
      print_msg "" ""
    fi
  fi

  if ! $RUNS_ON_TRAVIS; then
      print_msg "" "For detailed problem descriptions, consult the tagged messages above."
      print_msg "" "Tags may be [VERA], [CPPC], [DIFF], [COPY], [NAME] and [PEP8]."
  fi
  exit 1
else
  print_msg "" ""
  print_msg "MSGBLD0220: " "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
  print_msg "MSGBLD0220: " "+               STATIC CODE ANALYSIS TERMINATED SUCCESSFULLY !                +"
  print_msg "MSGBLD0220: " "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"
  print_msg "" ""
fi
