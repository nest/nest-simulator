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
# Usage: check_iwyu.sh [--changed <git-rev>] <build-dir> [<source-dir-or-file> ...]
#
# <build-dir> must contain compile_commands.json, i.e. NEST must have been
# configured with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON. Configuring is enough;
# nothing needs to be built. Without further arguments, nestkernel/, models/
# and libnestutil/ are checked.
#
# With --changed <git-rev>, only the C++ files in nestkernel/, models/ and
# libnestutil/ that differ from <git-rev> (including uncommitted changes) are
# checked: a changed foo.cpp, foo.h or foo_impl.h is checked through foo.cpp;
# a header without its own .cpp is checked within one file that includes it.
#
# Environment variables:
#   IWYU_JOBS             number of parallel IWYU processes [default: nproc]
#   IWYU_LOG              file for the complete IWYU output [default: <build-dir>/iwyu.log]
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

changed_rev=""
if [ "$1" = "--changed" ]; then
    changed_rev="${2:?--changed needs a git revision}"
    shift 2
fi

NEST_SOURCE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$(cd "$1" && pwd)"
shift

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "error: $BUILD_DIR/compile_commands.json not found; configure NEST with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON" >&2
    exit 2
fi

check_dirs=(nestkernel models libnestutil)
extra_args=()

if [ -n "$changed_rev" ]; then
    # Translation units known to the compilation database.
    declare -A is_tu=()
    while IFS= read -r f; do
        is_tu["$f"]=1
    done < <(python3 -c 'import json, sys; print("\n".join(e["file"] for e in json.load(open(sys.argv[1]))))' \
                     "$BUILD_DIR/compile_commands.json")

    declare -A selected=()
    pathspecs=()
    for d in "${check_dirs[@]}"; do
        pathspecs+=("$d/*.h" "$d/*.cpp")
    done
    while IFS= read -r f; do
        case "$f" in
            *.cpp)
                tu="$NEST_SOURCE/$f"
                ;;
            *.h)
                base="${f%.h}"
                tu="$NEST_SOURCE/${base%_impl}.cpp"
                if [ -z "${is_tu[$tu]:-}" ]; then
                    # Header without its own .cpp: check it within the first file including it.
                    tu="$(grep -lF "#include \"$(basename "$f")\"" "${!is_tu[@]}" 2>/dev/null | sort | head -n 1 || true)"
                    [ -n "$tu" ] && extra_args+=(-Xiwyu "--check_also=*/$f")
                fi
                ;;
        esac
        if [ -n "${tu:-}" ] && [ -n "${is_tu[$tu]:-}" ]; then
            selected["$tu"]=1
        else
            echo "note: no translation unit found to check $f"
        fi
    done < <(git -C "$NEST_SOURCE" diff --name-only --diff-filter=d "$changed_rev" -- "${pathspecs[@]}")

    if [ ${#selected[@]} -eq 0 ]; then
        echo "IWYU: no C++ files changed relative to $changed_rev in ${check_dirs[*]}; nothing to check."
        if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
            printf '## include-what-you-use\n\nNo C++ files changed in %s.\n' "${check_dirs[*]}" >> "$GITHUB_STEP_SUMMARY"
        fi
        exit 0
    fi
    mapfile -t targets < <(printf '%s\n' "${!selected[@]}" | sort)
    echo "Checking ${#targets[@]} translation unit(s) affected by changes relative to $changed_rev."
elif [ $# -gt 0 ]; then
    targets=("$@")
else
    targets=()
    for d in "${check_dirs[@]}"; do
        targets+=("$NEST_SOURCE/$d")
    done
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
    "${extra_args[@]}" \
    2>&1 | grep -v "no private include name for @headername mapping" > "$IWYU_LOG" || true

# Report paths relative to the source directory. IWYU suggests <stddef.h> for
# size_t, but NEST uses <cstddef>, which IWYU accepts as well.
sed -i -e "s|$NEST_SOURCE/||g" -e "s|add '#include <stddef.h>'|add '#include <cstddef>'|" "$IWYU_LOG"

# A header checked from several translation units is reported once per unit.
findings="$(grep -E '^[^ ]+:[0-9]+:[0-9]+: warning: ' "$IWYU_LOG" | awk '!seen[$0]++' || true)"
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
