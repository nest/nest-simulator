#!/bin/bash
#
# list_examples.sh
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

# set bash strict mode
set -euo pipefail
IFS=$' \n\t'

echo ">>> Longest running examples:"
egrep -o "real: [^,]+" example_logs/*/meta.yaml | sed -e 's/:real://' | sort -k2 -r | head -n 20 || true;

echo ">>> multiple run statistics"
for x in example_logs/*/meta.yaml; do
	name="$(basename "$(dirname "$x")")"
	res="$(grep "result:" "$x" | sort | uniq -c | tr "\n" "\t" | sed -e 's/failed/\x1b[1;31m\0\x1b[0m/g' -e 's/success/\x1b[1;32m\0\x1b[0m/g')"
	warn=""
	if grep "fail" <<<"$res" >/dev/null && grep "success" <<<"$res" >/dev/null; then
		warn='\033[01;33m'
	fi
	echo -e "$res\t$warn$name\033[0m"
done
echo "<<< done"
