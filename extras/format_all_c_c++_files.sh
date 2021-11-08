#! /usr/bin/env bash

# With this script you can easily format all C/C++ files contained in
# any (sub)directory of NEST. Internally it uses clang-format to do
# the actual formatting. You can give a different clang-format command,
# e.g. by executing `CLANG_FORMAT=clang-format-3.6 ./format_all_c_c++_files.sh`.
# By default the script starts at the current working directory ($PWD), but
# supply a different starting directory as the first argument to the command.
CLANG_FORMAT=${CLANG_FORMAT-clang-format-3.6}
CLANG_FORMAT_FILE=${CLANG_FORMAT_FILE-.clang-format}

# Drop files that should not be checked
FILES_TO_IGNORE="pynestkernel.cpp"
DIRS_TO_IGNORE="thirdparty"

# Recursively process all C/C++ files in all sub-directories.
function process_dir {
  dir=$1
  echo "Process directory: $dir"

  if [[ " $DIRS_TO_IGNORE " =~ .*[[:space:]]${dir##*/}[[:space:]].* ]]; then
    echo "   Directory explicitly ignored."
    return
  fi

  for f in $dir/*; do
    if [[ -d $f ]]; then
      # Recursively process sub-directories.
      process_dir $f
    else
      ignore_file=0

      for FILE_TO_IGNORE in $FILES_TO_IGNORE; do
        if [[ $f == *$FILE_TO_IGNORE* ]]; then
          ignore_file=1
          break
        fi
      done

      if [ $ignore_file == 1 ] ; then
        continue
      fi

      case $f in
        *.cpp | *.cc | *.c | *.h | *.hpp )
          # Format C/C++ files.
          echo " - Format C/C++ file: $f"
          $CLANG_FORMAT -i $f
          ;;
        * )
          # Ignore all other files.
      esac
    fi
  done
}

function help_output {
  echo "The $CLANG_FORMAT_FILE requires clang-format version 3.6 or later."
  echo "Use like: [CLANG_FORMAT=<clang-format-3.6>] ./extras/`basename $0` [start folder, defaults to '$PWD']"
  exit 0
}

if [[ ! -f $CLANG_FORMAT_FILE ]]; then
  echo "Cannot find $CLANG_FORMAT_FILE file. Please start '`basename $0`' from the NEST base source directory."
  help_output
fi

if [[ $# -eq 0 ]]; then
  # Start with current directory.
  startdir=$PWD
elif [[ $# -eq 1 ]]; then
  if [[ -d $1 ]]; then
    # Start with given directory.
    startdir=$1
  else
    # Not a directory.
    help_output
  fi
else
  # Two or more arguments...
  help_output
fi

# Start formatting the $startdir and all subdirectories
process_dir $startdir
