#!/bin/bash
#
# run_examples.sh
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

NEST_CMD=`which nest`
if [ $? != 0 ] ; then
    echo "ERROR: command 'nest' not found. Please make sure PATH is set correctly"
    echo "       by sourcing the script nest_vars.sh from your NEST installation."
    exit 1
fi

python -c "import nest" >/dev/null 2>&1
if [ $? != 0 ] ; then
    echo "ERROR: PyNEST is not available. Please make sure PYTHONPATH is set correctly"
    echo "       by sourcing the script nest_vars.sh from your NEST installation."
    exit 1
fi

FAILURES=0


# Find all examples that have a line containing "autorun=true"
# The examples can be found in subdirectory nest and in the 
# examples installation path.
if [ -d "nest/" ] ; then
    EXAMPLES=$(grep -rl --include=\*\.sli 'autorun=true' nest/)
else
    EXAMPLES=$(grep -rl --include=\*\.sli 'autorun=true' examples/)
fi

if test -n "$SKIP_LIST"; then
    EXAMPLES=$(echo $EXAMPLES | tr ' ' '\n' | grep -vE $SKIP)
fi

time_format="    TIME:    real: %E, user: %U, sys: %S\n\
    MEMORY:  total: %K, max rss: %M"

basedir=$PWD

START=$SECONDS
for i in $EXAMPLES ; do

    cd $(dirname $i)

    workdir=$PWD
    example=$(basename $i)

    ext=$(echo $example | cut -d. -f2)

    if [ $ext = sli ] ; then
        runner=nest
    elif [ $ext = py ] ; then
        runner=python
    fi

    output_dir=$basedir/example_logs/$example
    logfile=$output_dir/output.log
    mkdir -p $output_dir
    
    echo ">>> RUNNING: $workdir/$example"
    echo "    LOGFILE: $logfile"

    export NEST_DATA_PATH=$output_dir
    /usr/bin/time -f "$time_format" --quiet sh -c "$runner $example >$logfile 2>&1"

    if [ $? != 0 ] ; then
        echo "    FAILURE!"
        FAILURES=$(( $FAILURES + 1 ))
        OUTPUT=$(printf "        %s\n        %s\n" "$OUTPUT" "$workdir/$example")
    else
        echo "    SUCCESS!"
    fi
    echo

    unset NEST_DATA_PATH
    cd $basedir

done
ELAPSED_TIME=$(($SECONDS - $START))

echo ">>> RESULTS: $FAILURES failed /" $(echo $EXAMPLES | wc -w) " total"
echo ">>> TOTAL TIME: $(($ELAPSED_TIME/60)) min $(($ELAPSED_TIME%60)) sec."

if [ "x$OUTPUT" != "x" ] ; then
    echo ">>> Failed examples:"
    echo "$OUTPUT"
    echo ""
    exit 1
fi

exit 0
