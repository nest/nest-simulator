#!/usr/bin/env bash

# This script runs both mpi tests, compares their output and reports.
# It should only report equal files.

# nest executable
nest=$1

# process numbers and tests
ref_proc_num=1
test_proc_nums=(2 4)
proc_nums=($ref_proc_num ${test_proc_nums[*]})

tests=('mini_brunel_ps_csvp' 'mini_brunel_ps_rr')

# remove old output
rm -rf ${proc_nums[*]}

# run tests
for tcase in ${tests[*]} 
do
  for n in ${proc_nums[*]}
  do
    mkdir -p $n/$tcase
    cd $n/$tcase
    mpirun -np $n $nest ../../test_${tcase}.sli > /dev/null 
    cd ../..
  done
done

all_ok="true"
for tn in ${test_proc_nums[*]}
do
  diff -qr $ref_proc_num $tn || all_ok="false"
done

# remove output
rm -rf ${proc_nums[*]}

if test x${all_ok} = "xtrue" ; then
  echo "TESTS PASSED"
  exit 0
else
  echo "******* TESTS FAILED *******"
  exit 25
fi

  