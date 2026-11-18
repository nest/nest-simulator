#!/usr/bin/env bash
#
# run_in_container.sh
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

# Run build_support/check_iwyu.sh in a pinned Ubuntu container.
#
# Usage: build_support/iwyu/run_in_container.sh [--changed <git-rev>] [--fix] [<source-dir-or-file> ...]
#
# IWYU's advice depends on the IWYU version and the standard library, so a
# check on one machine can disagree with another. This script runs the check
# in the image below (IWYU 0.23, GCC 15's libstdc++), the same way as the
# on-demand include-what-you-use CI workflow, so results agree everywhere.
# Needs Docker or Podman. The first run downloads the image; every run installs
# the required packages, which takes a few minutes.
#
# The arguments are passed to check_iwyu.sh. NEST is configured in
# build/iwyu-container (OpenMP, MPI, GSL, Boost and libltdl on); the log is
# build/iwyu-container/iwyu.log.
#
# Environment variables:
#   IWYU_IMAGE        container image [default: ubuntu:26.04]
#   CONTAINER_ENGINE  docker or podman [default: whichever is found first]

set -euo pipefail

NEST_SOURCE="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMAGE="${IWYU_IMAGE:-ubuntu:26.04}"
ENGINE="${CONTAINER_ENGINE:-$(command -v docker || command -v podman || true)}"
if [ -z "$ENGINE" ]; then
    echo "error: neither docker nor podman found; install one or run build_support/check_iwyu.sh directly" >&2
    exit 2
fi

tty=()
[ -t 1 ] && tty=(-t)
exec "$ENGINE" run --rm "${tty[@]}" \
    -v "$NEST_SOURCE:/nest" -w /nest \
    -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" \
    -e IWYU_ANNOTATE_FILES="${IWYU_ANNOTATE_FILES:-}" \
    "$IMAGE" bash build_support/iwyu/container_check.sh "$@"
