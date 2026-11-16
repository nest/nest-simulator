#!/usr/bin/env bash
#
# check_iwyu.sh
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

# Run include-what-you-use (IWYU) on the NEST C++ sources.
#
# Usage: check_iwyu.sh <build-dir> [<source-dir-or-file> ...]
#
# <build-dir> must contain compile_commands.json, i.e. NEST must have been
# configured with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON. Configuring is enough;
# nothing needs to be built. Without further arguments, nestkernel/, models/
# and libnestutil/ are checked.
#
# Environment variables:
#   IWYU_JOBS             number of parallel IWYU processes [default: nproc]
#   IWYU_LOG              file for the filtered IWYU output [default: <build-dir>/iwyu.log]
#   IWYU_ANNOTATE_FILES   whitespace-separated list of source-relative paths for
#                         which GitHub Actions annotations are printed (CI only)
#   GITHUB_STEP_SUMMARY   if set (GitHub Actions), a Markdown summary is appended
#
# Exit status: 0 if IWYU has no findings, 1 if it has findings, 2 if IWYU could
# not analyse all files (e.g. a header was not found).

set -euo pipefail

if [ $# -lt 1 ]; then
    sed -n '/^# Usage:/,/^# Exit status/p' "$0" | sed 's/^# \{0,1\}//'
    exit 2
fi

NEST_SOURCE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$(cd "$1" && pwd)"
shift

if [ $# -gt 0 ]; then
    targets=("$@")
else
    targets=("$NEST_SOURCE/nestkernel" "$NEST_SOURCE/models" "$NEST_SOURCE/libnestutil")
fi

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "error: $BUILD_DIR/compile_commands.json not found; configure NEST with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON" >&2
    exit 2
fi

IWYU_TOOL="$(command -v iwyu_tool || command -v iwyu_tool.py || true)"
if [ -z "$IWYU_TOOL" ]; then
    echo "error: iwyu_tool (or iwyu_tool.py) not found on PATH; install include-what-you-use" >&2
    exit 2
fi

IWYU_JOBS="${IWYU_JOBS:-$(nproc)}"
IWYU_LOG="${IWYU_LOG:-$BUILD_DIR/iwyu.log}"

echo "Running $(include-what-you-use --version | head -n 1) with $IWYU_JOBS jobs ..."

# - clang-warning output gives "file:line:col: warning: ..." lines.
# - --keep: config.h is often needed only for #ifdef HAVE_..., which IWYU does
#   not count as a use; removing it would silently change the build.
# - The grep drops IWYU's own complaints about libstdc++'s @headername comments.
"$IWYU_TOOL" -p "$BUILD_DIR" -j "$IWYU_JOBS" -o clang-warning "${targets[@]}" -- \
    -Xiwyu --mapping_file="$NEST_SOURCE/build_support/iwyu/nest.imp" \
    -Xiwyu --cxx17ns \
    -Xiwyu --keep='*config.h*' \
    2>&1 | grep -v "no private include name for @headername mapping" > "$IWYU_LOG" || true

# Report paths relative to the source directory.
sed -i "s|$NEST_SOURCE/||g" "$IWYU_LOG"

findings="$(grep -E '^[^ ]+:[0-9]+:[0-9]+: warning: ' "$IWYU_LOG" || true)"
n_findings="$(printf '%s' "$findings" | grep -c . || true)"
n_files="$(printf '%s' "$findings" | cut -d: -f1 | sort -u | grep -c . || true)"
# Missing headers, IWYU crashes (assertions) and mapping-file errors.
n_fatal="$(grep -cE 'fatal error:|Assertion failed|^error:|: error: ' "$IWYU_LOG" || true)"

echo "IWYU: $n_findings findings in $n_files files ($n_fatal fatal errors). Full output: $IWYU_LOG"

if [ -n "$findings" ]; then
    echo
    echo "Files with the most findings:"
    printf '%s\n' "$findings" | cut -d: -f1 | sort | uniq -c | sort -rn | head -n 20
fi

if [ -n "${IWYU_ANNOTATE_FILES:-}" ] && [ -n "$findings" ]; then
    for f in $IWYU_ANNOTATE_FILES; do
        printf '%s\n' "$findings" | awk -v f="$f:" 'index($0, f) == 1' | while IFS= read -r line; do
            file="${line%%:*}"
            rest="${line#*:}"
            lineno="${rest%%:*}"
            msg="${line#*warning: }"
            echo "::warning file=$file,line=$lineno,title=include-what-you-use::$msg"
        done
    done
fi

if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
    {
        echo "## include-what-you-use"
        echo
        echo "**$n_findings findings in $n_files files** ($n_fatal fatal errors)."
        echo "This check is informational and does not block merging."
        echo "The full log is attached to this workflow run as the \`iwyu-log\` artifact."
        echo
        if [ -n "$findings" ]; then
            echo "<details><summary>Findings per file</summary>"
            echo
            echo '```'
            printf '%s\n' "$findings" | cut -d: -f1 | sort | uniq -c | sort -rn
            echo '```'
            echo
            echo "</details>"
        fi
    } >> "$GITHUB_STEP_SUMMARY"
fi

if [ "$n_fatal" -gt 0 ]; then
    grep -B4 -E 'fatal error:|Assertion failed|^error:|: error: ' "$IWYU_LOG" | head -n 40 >&2
    exit 2
fi
[ "$n_findings" -eq 0 ]
