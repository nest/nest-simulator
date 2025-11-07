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

if ! command -v nest; then
    echo "ERROR: command 'nest' not found. Please make sure PATH is set correctly"
    echo "       by sourcing the script nest_vars.sh from your NEST installation."
    exit 1
fi

if ! python3 -c "import nest" >/dev/null 2>&1; then
    echo "ERROR: PyNEST is not available. Please make sure PYTHONPATH is set correctly"
    echo "       by sourcing the script nest_vars.sh from your NEST installation."
    exit 1
fi

# set bash strict mode
set -euo pipefail
set -x
IFS=$' \n\t'

# current directory where all results are save is "basedir"
basedir="$PWD"
# source tree where all input scripts/data can be found is "sourcedir"
sourcedir="$(realpath "$(dirname "$0")/..")"

declare -a EXAMPLES
if [ "${#}" -eq 0 ]; then
    # Find all examples that have a line containing "autorun=true"
    # The examples can be found in subdirectory nest and in the
    # examples installation path.
    EXAMPLELIST="$(mktemp examplelist.XXXXXXXXXX)"
    if [ -d "nest/" ] ; then
        grep -rl --include='*.sli' 'autorun=true' "${sourcedir}/nest/" >>"$EXAMPLELIST" || true
    else
        grep -rl --include='*.sli' 'autorun=true' "${sourcedir}/examples/" >>"$EXAMPLELIST" || true
    fi
    find "${sourcedir}/pynest/examples" -name '*.py' >>"$EXAMPLELIST" || true
    readarray -t EXAMPLES <"$EXAMPLELIST"  # append each example found above
    rm "$EXAMPLELIST"
else
    EXAMPLES+=( "${@}" )
fi

for i in $(seq 0 $(( ${#EXAMPLES[@]}-1))); do
    if echo "${EXAMPLES[$i]}" | grep -vE "${SKIP_LIST}" >/dev/null; then
        unset "EXAMPLES['$i']"
    fi
done

# turn off plotting to the screen and waiting for input
MPLCONFIGDIR="$(pwd)/matplotlib/"
export MPLCONFIGDIR

INFO_OS="$(uname -s)"

# find "time" command
if test "${INFO_OS:-}" = "Darwin"; then
    if which gtime >/dev/null; then
        # https://www.man7.org/linux/man-pages/man1/time.1.html
        #
        # Silence the warning, as we explicitly do the aliassing to have a
        # defined time-binary to call. No need to re-lookup each call.
        # SC2139 (warning): This expands when defined, not when used. Consider escaping.
        # shellcheck disable=SC2139
        alias time="$(which gtime) -f '  time: {real: %E, user: %U, system: %S}\n  memory: {total: %K, max_rss: %M}' --quiet"
    else
        if command -v gtime >/dev/null; then
            alias time="\$(command -v gtime) -f '  time: {real: %E, user: %U, system: %S}\n  memory: {total: %K, max_rss: %M}' --quiet"
        elif command -v time >/dev/null; then
            # bash built-in time does not have memory information and uses TIMEFORMAT env variable
            TIMEFORMAT="  time: {real: %E, user: %U, system: %S}"
            export TIMEFORMAT
        else
            echo "'time' does not work on macOS. Try 'brew install gnu-time' or provide a compatible command by different means."
            exit 1;
        fi
    fi
else
    # bash built-in time does not have memory information and uses TIMEFORMAT env variable
    TIMEFORMAT="  time: {real: %E, user: %U, system: %S}"
    export TIMEFORMAT
    if ! command -v time >/dev/null; then
        echo "could not determine a 'time' command. aborting"
        exit 1;
    fi
fi

FAILURES=0
START="${SECONDS}"
for i in "${EXAMPLES[@]}"; do

    cd "$(dirname "$i")"

    workdir="$PWD"
    example="$(basename "$i")"

    ext="$(echo "$example" | cut -d. -f2)"

    if [ "${ext}" = "sli" ] ; then
        runner="nest"
    elif [ "${ext}" = "py" ] ; then
        runner="python3"
    fi

    output_dir="$basedir/example_logs/$example"
    logfile="$output_dir/output.log"
    metafile="$output_dir/meta.yaml"
    mkdir -pv "$output_dir"

    echo ">>> RUNNING: $workdir/$example"
    echo "    LOGFILE: $logfile"
    {
        echo "- script: '$workdir/$example'"
        echo "  output_dir: '$output_dir'"
        echo "  log: '$logfile'"
    } >>"$metafile"

    export NEST_DATA_PATH="$output_dir"
    touch .start_example
    #sleep 1  # why was this needed?!
    set +e
    time sh -c "'$runner' '$example' >'$logfile' 2>&1" 2>&1 | tee -a "$metafile"
    ret=$?
    set -e

    outfiles=false
    find . -newer .start_example | while read -r file; do
        if ! $outfiles; then
            echo "  output_files:" >>"$metafile"
            outfiles="true"
        fi
        echo "  - '$file'" >>"$metafile"
    done
    echo "  return_code: $ret" >>"$metafile"
    if [ $ret != 0 ] ; then
        echo "    FAILURE!"
        echo "  result: failed" >>"$metafile"
        FAILURES="$(( FAILURES + 1 ))"
        OUTPUT="$(printf "        %s\n        %s\n" "${OUTPUT:-}" "$workdir/$example")"
    else
        echo "    SUCCESS!"
        echo "  result: success" >>"$metafile"
    fi
    echo

    unset NEST_DATA_PATH
    cd "$basedir"
done
ELAPSED_TIME="$(( SECONDS - START ))"

echo ">>> Longest running examples:"
grep -Eo "real: [^,]+" example_logs/*/meta.yaml | sed -e 's/:real://' | sort -k2 -rg | head -n 15 || true

echo ">>> RESULTS: ${FAILURES} failed /$(echo "${EXAMPLES[*]}" | wc -w) total"
echo ">>> TOTAL TIME: $((ELAPSED_TIME/60)) min $((ELAPSED_TIME%60)) sec."

if [ -n "${OUTPUT+x}" ] ; then
    echo ">>> Failed examples:"
    echo "${OUTPUT}"
    echo ""
    exit 1
fi

exit 0
