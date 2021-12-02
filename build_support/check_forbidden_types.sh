#!/bin/bash
#
# check_forbidden_types.sh
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

# This script scans for forbidden types and fails if any are used.

forbidden_types="double_t"
used_forbidden_types=""

for t in $forbidden_types; do
    scan_result=`grep $t -r -o --include=*.{h,cc,cpp} $NEST_SOURCE | sed 's/:/: /'`
    if [ -n "$scan_result" ]; then
        used_forbidden_types=$used_forbidden_types$'\n'$scan_result
    fi
done

echo "$used_forbidden_types"
