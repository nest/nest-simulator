#!/usr/bin/env bash
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
set -euo pipefail

forbidden_types="double_t"
used_forbidden_types=""
NEST_SOURCE="${NEST_SOURCE:-.}"

for t in $forbidden_types; do
    scan_result="$(grep "$t" -rHn -o --include=*.{h,cc,cpp} "$NEST_SOURCE" || true)"
    if [ -n "$scan_result" ]; then
        used_forbidden_types="$used_forbidden_types$'\n'$scan_result"
    fi
done

test -z "$used_forbidden_types" || { # return success if no forbidden types where used or print the corresponding list and error out
    echo "$used_forbidden_types"
    exit 1
}
