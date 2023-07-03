#!/bin/bash

# version_info.sh
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

# This script extracts version control information to be used in
# cmake/NestVersionInfo.cmake

# If we can't run git at all, set everything to "unknown"
if ! command -v git > /dev/null 2>&1; then
  echo unknown\;unknown\;unknown
  exit 0
fi

HASH=$(git rev-parse HEAD)

# Might fail if not on a branch, or no remote tracking branch is set
BRANCH_REMOTE=$(git rev-parse --abbrev-ref --symbolic-full-name @{u} 2>&1)
if [ $? -eq 0 ]; then
  REMOTE=$(echo ${BRANCH_REMOTE} | cut -d\/ -f1)
else
  REMOTE="unknown"
fi

# Might fail if we are not on a branch (i.e. in 'detached HEAD' mode)
BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [ ! $? -eq 0 ] || [ ${BRANCH} = "HEAD" ]; then
  BRANCH="unknown"
fi

# Printing with semicolons as separators, as this will be interpreted as
# a list by cmake
echo ${HASH}\;${BRANCH}\;${REMOTE}
