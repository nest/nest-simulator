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

FAILURES=0

# The extension will be the third field if separated by .
EXAMPLES=$(find ${SEARCH_DIR:-./examples} -type f -name \*.py -o -name \*.sli | sort -t. -k3)

for i in $EXAMPLES ; do

    ext=$(basename $i | cut -d. -f2)

    if [ $ext = sli ] ; then
        runner=nest
    elif [ $ext = py ] ; then
        runner=python
    fi

    echo ">>> RUNNING: $i"

    set +e
    $runner $i

    if [ $? != 0 ] ; then
        echo ">>> FAILURE: $i"
        FAILURES=$(( $FAILURES + 1 ))
        OUTPUT=$(printf "        %s\n        %s\n" "$OUTPUT" "$i")
    else
        echo ">>> SUCCESS: $i"
    fi
    set -e

done

echo ">>> RESULTS: $FAILURES /" $(echo $EXAMPLES | wc -w) "(failed / total)"

if [ "x$OUTPUT" != "x" ] ; then
    echo ">>> Failed examples:"
    echo "$OUTPUT"
    echo ""
    exit 1
fi

exit 0
