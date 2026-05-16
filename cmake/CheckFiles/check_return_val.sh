#!/usr/bin/env sh

# We're only interested in the return value of the script, so x is dummy only.
# SC2034 (warning): x appears unused. Verify use (or export if used externally).
# shellcheck disable=SC2034
x="$("$1")"
exit $?
