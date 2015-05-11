#!/usr/bin/env bash

# This script runs all tests in this directory and then compares the
# output with the reference output using diff.
# It takes the NEST executable as argument.

# executable
NEST=$1

mkdir -p output

# Remove old stuff from output directory
rm -rf output/*

# Run tests
find testscripts -name '*.sli' -exec $NEST -c '({}) (run_test.sli) run' \;

# Perform diff
diff -r -x .svn output reference && result="ok" || result="error"

if [ $result = "ok" ] ; then
  echo "OK: Tests reproduced reference results"
  rm -r output/*
else
  echo "ERROR: Tests produced different output from reference!!!"
fi