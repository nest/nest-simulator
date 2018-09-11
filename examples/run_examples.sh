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

set -e

SKIP_LIST="
    MyModule/sli/example.sli
    Potjans_2014/spike_analysis.py
    Potjans_2014/user_params.sli
    ReadData_demo.sli
    music/
    nestrc.sli
    neuronview.py
    plot_tsodyks_depr_fac.py
    plot_tsodyks_shortterm_bursts.py
"

FAILURES=0

# Trim leading and trailing whitespace and make gaps exactly one space large
SKIP_LIST=$(echo $SKIP_LIST | sed -e 's/ +/ /g' -e 's/^ *//' -e 's/ *$//')

# Create a regular expression for grep that removes the excluded files
case "$SKIP_LIST" in  
    *\ * ) # We have spaces in the list
        SKIP='('$(echo $SKIP_LIST | tr ' ' '|' )')' ;;
    *)
        SKIP=$SKIP_LIST ;;
esac

# Find all examples in the installation directory
EXAMPLES=$(find ${SEARCH_DIR:-./examples} -type f -name \*.py -o -name \*.sli | sort -t. -k3)

if test -n "$SKIP_LIST"; then
    EXAMPLES=$(echo $EXAMPLES | tr ' ' '\n' | grep -vE $SKIP)
fi

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
        runner=$(nest-config --python-executable)
    fi

    echo ">>> RUNNING: $workdir/$example"

    set +e
    $runner $example

    if [ $? != 0 ] ; then
        echo ">>> FAILURE: $workdir/$example"
        FAILURES=$(( $FAILURES + 1 ))
        OUTPUT=$(printf "        %s\n        %s\n" "$OUTPUT" "$workdir/$example")
    else
        echo ">>> SUCCESS: $example"
    fi
    echo
    set -e

    cd $basedir

done
ELAPSED_TIME=$(($SECONDS - $START))

echo ">>> RESULTS: $FAILURES /" $(echo $EXAMPLES | wc -w) "(failed / total)"
echo ">>> TIME: $(($ELAPSED_TIME/60)) min $(($ELAPSED_TIME%60)) sec."

if [ "x$OUTPUT" != "x" ] ; then
    echo ">>> Failed examples:"
    echo "$OUTPUT"
    echo ""
    exit 1
fi

exit 0
