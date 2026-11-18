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
# Usage: check_iwyu.sh [--changed <git-rev>] [--fix] <build-dir> [<source-dir-or-file> ...]
#
# <build-dir> must contain compile_commands.json, i.e. NEST must have been
# configured with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON. Configuring is enough;
# nothing needs to be built. Without further arguments, nestkernel/, models/
# and libnestutil/ are checked.
#
# A header foo.h is analysed together with foo.cpp. Every other header
# (header-only code, *_impl.h) gets a stub translation unit that only includes
# it, see build_support/iwyu/add_header_tus.py; this also checks that every
# header compiles on its own.
#
# With --changed <git-rev>, only the C++ files in nestkernel/, models/ and
# libnestutil/ that differ from <git-rev> (including uncommitted changes) are
# checked.
#
# With --fix, IWYU's advice is applied with IWYU's fix_includes.py (fix_include
# on Debian/Ubuntu) and the changed files are formatted with clang-format. The
# advice for one file can change once others are fixed, so repeat until the
# check reports no findings. Then build NEST with and without the optional
# libraries: IWYU sees only the code of the configured features.
#
# Results depend on the IWYU version and the standard library. To get the same
# results as everyone else, run build_support/iwyu/run_in_container.sh, which
# uses a pinned Ubuntu image.
#
# Environment variables:
#   IWYU_JOBS             number of parallel IWYU processes [default: nproc]
#   IWYU_LOG              file for the complete IWYU output [default: <build-dir>/iwyu.log]
#   IWYU_ANNOTATE_FILES   whitespace-separated list of source-relative paths for
#                         which GitHub Actions annotations are printed (CI only)
#   GITHUB_STEP_SUMMARY   if set (GitHub Actions), a Markdown summary is appended
#   CLANG_FORMAT          clang-format executable used by --fix [default: clang-format]
#
# Exit status: 0 if IWYU has no findings, 1 if it has findings (also with --fix,
# where they have then been applied), 2 if IWYU could not analyse all files
# (e.g. a header was not found).

set -euo pipefail

if [ $# -lt 1 ]; then
    sed -n '/^# Usage:/,/^# Exit status/p' "$0" | sed 's/^# \{0,1\}//'
    exit 2
fi

changed_rev=""
fix=0
while [ $# -gt 0 ]; do
    case "$1" in
        --changed)
            changed_rev="${2:?--changed needs a git revision}"
            shift 2
            ;;
        --fix)
            fix=1
            shift
            ;;
        *)
            break
            ;;
    esac
done
if [ $# -lt 1 ]; then
    echo "error: no build directory given" >&2
    exit 2
fi

NEST_SOURCE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$(cd "$1" && pwd)"
shift

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "error: $BUILD_DIR/compile_commands.json not found; configure NEST with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON" >&2
    exit 2
fi

check_dirs=(nestkernel models libnestutil)

# Compilation database including the header stubs.
IWYU_DB="$BUILD_DIR/iwyu"
rm -rf "$IWYU_DB"
mkdir -p "$IWYU_DB"
python3 "$NEST_SOURCE/build_support/iwyu/add_header_tus.py" "$BUILD_DIR" "$NEST_SOURCE" "$IWYU_DB" "${check_dirs[@]}"

if [ -n "$changed_rev" ]; then
    # Translation units known to the compilation database.
    declare -A is_tu=()
    while IFS= read -r f; do
        is_tu["$f"]=1
    done < <(python3 -c 'import json, sys; print("\n".join(e["file"] for e in json.load(open(sys.argv[1]))))' \
                     "$IWYU_DB/compile_commands.json")

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
                tu="$NEST_SOURCE/${f%.h}.cpp"
                [ -n "${is_tu[$tu]:-}" ] || tu="$IWYU_DB/${f%.h}.iwyu.cpp"
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
    targets+=("$IWYU_DB")
fi

IWYU_TOOL="$(command -v iwyu_tool || command -v iwyu_tool.py || true)"
if [ -z "$IWYU_TOOL" ]; then
    echo "error: iwyu_tool (or iwyu_tool.py) not found on PATH; install include-what-you-use" >&2
    exit 2
fi

IWYU_JOBS="${IWYU_JOBS:-$(nproc)}"
IWYU_LOG="${IWYU_LOG:-$BUILD_DIR/iwyu.log}"
IWYU_RAW="$BUILD_DIR/iwyu.raw"

echo "Running $(include-what-you-use --version | head -n 1) with $IWYU_JOBS jobs ..."

# Convert IWYU's own output format, which all IWYU versions produce, into
# "file:line:col: warning: ..." lines (iwyu_tool's clang-warning format exists
# only from IWYU 0.22 on). Compiler errors in a header stub mean that the header
# does not compile on its own; they are reported as findings, not as failures of
# IWYU. Other compiler diagnostics are passed through unchanged.
iwyu_to_warnings() {
    awk '
        /^In file included from .*[.]iwyu[.]cpp:/ { stub = 1 }
        /^$/                                       { stub = 0 }
        stub && /: error: /                        { sub(/: error: /, ": warning: does not compile on its own: ") }
        / should add these lines:$/    { mode = "add"; file = $0; sub(/ should add these lines:$/, "", file); next }
        / should remove these lines:$/ { mode = "remove"; file = $0; sub(/ should remove these lines:$/, "", file); next }
        /^The full include-list for /  { mode = "list"; next }
        /^---$/                        { mode = ""; next }
        /^\(.* has correct #includes\/fwd-decls\)$/ {
            f = $0; sub(/^\(/, "", f); sub(/ has correct #includes\/fwd-decls\)$/, "", f)
            print f ":1:1: note: #includes/fwd-decls are correct"; next
        }
        mode == "add" && NF {
            item = $0; reason = ""
            if (match(item, / +\/\/ for /)) { reason = substr(item, RSTART + RLENGTH); item = substr(item, 1, RSTART - 1) }
            sub(/ +$/, "", item)
            printf "%s:1:1: warning: add '\''%s'\''%s\n", file, item, (reason != "" ? " (for " reason ")" : "")
            next
        }
        mode == "remove" && NF {
            item = $0; sub(/^- /, "", item); lineno = 1
            if (match(item, / +\/\/ lines? [0-9]+/)) {
                tail = substr(item, RSTART); item = substr(item, 1, RSTART - 1)
                match(tail, /[0-9]+/); lineno = substr(tail, RSTART, RLENGTH)
            }
            printf "%s:%s:1: warning: superfluous '\''%s'\''\n", file, lineno, item
            next
        }
        mode != "" { next }
        { print }
    '
}

# - --keep: config.h is often needed only for #ifdef HAVE_..., which IWYU does
#   not count as a use; removing it would silently change the build. IWYU cannot
#   tell where the template definitions in *_impl.h are instantiated, so it must
#   not remove those includes either.
# - The grep drops IWYU's own complaints about libstdc++'s @headername comments.
# - IWYU suggests <stddef.h> for size_t, but NEST uses <cstddef>, which IWYU
#   accepts as well.
"$IWYU_TOOL" -p "$IWYU_DB" -j "$IWYU_JOBS" -o iwyu "${targets[@]}" -- \
    -Xiwyu --mapping_file="$NEST_SOURCE/build_support/iwyu/nest.imp" \
    -Xiwyu --cxx17ns \
    -Xiwyu --keep='*config.h*' \
    -Xiwyu --keep='*_impl.h*' \
    2>&1 | grep -v "no private include name for @headername mapping" |
    sed -e 's|#include <stddef.h>|#include <cstddef>|' > "$IWYU_RAW" || true
iwyu_to_warnings < "$IWYU_RAW" > "$IWYU_LOG"

# Report paths relative to the source directory.
sed -i -e "s|$IWYU_DB/||g" -e "s|$NEST_SOURCE/||g" "$IWYU_LOG"

# A header checked from several translation units is reported once per unit.
findings="$(grep -E '^[^ ]+:[0-9]+:[0-9]+: warning: ' "$IWYU_LOG" | grep -v '[.]iwyu[.]cpp:' | awk '!seen[$0]++' || true)"
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
        echo "$(include-what-you-use --version | head -n 1)."
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

if [ "$fix" -eq 1 ] && [ "$n_findings" -gt 0 ]; then
    FIX_INCLUDES="$(command -v fix_includes.py || command -v fix_include || true)"
    if [ -z "$FIX_INCLUDES" ]; then
        echo "error: fix_includes.py (or fix_include) not found on PATH" >&2
        exit 2
    fi
    CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
    required="$(sed -n 's/^ *CLANG_REQUIRE_VERSION: *//p' "$NEST_SOURCE/.github/workflows/nestbuildmatrix.yml" | head -n 1)"
    if ! "$CLANG_FORMAT" --version 2>/dev/null | grep -q "version ${required:-unknown}"; then
        echo "warning: NEST requires clang-format $required; $("$CLANG_FORMAT" --version 2>/dev/null | head -n 1 || echo "$CLANG_FORMAT not found")" >&2
    fi
    # Header stubs live in the build directory; only change NEST's own files.
    only_re="^$NEST_SOURCE/($(IFS='|'; echo "${check_dirs[*]}"))/"
    python3 "$FIX_INCLUDES" --nosafe_headers --nocomments --only_re="$only_re"         < "$IWYU_RAW" > "$BUILD_DIR/fix_includes.log" 2>&1 || true
    mapfile -t fixed < <(git -C "$NEST_SOURCE" diff --name-only -- "${check_dirs[@]}")
    if [ ${#fixed[@]} -gt 0 ]; then
        (cd "$NEST_SOURCE" && "$CLANG_FORMAT" -i "${fixed[@]}")
    fi
    echo
    echo "Applied IWYU's advice (log: $BUILD_DIR/fix_includes.log); ${#fixed[@]} file(s) in ${check_dirs[*]} now differ from the index."
    echo "Run the check again until it reports no findings, then build NEST with and without the optional libraries."
fi
[ "$n_findings" -eq 0 ]
