# NestVersionInfo.cmake
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

# Determine NEST version based on git branch
#
# This module defines
#  NEST_VERSION_BRANCH, the current git branch
#  NEST_VERSION_SUFFIX, set using -Dwith-version-suffix=<suffix>.
#  NEST_VERSION, the numeric version number plus the suffix
#  NEST_VERSION_GITHASH, the current git revision hash (empty for tarballs)
#  NEST_VERSION_STRING, the full NEST version string
#
# In release branches, the string "UNKNOWN" below has to be replaced
# with the proper version (e.g. "nest-2.18") in order to get the
# correct version number if building from tarballs.


macro(get_version_info)
   execute_process(
        COMMAND "git" "rev-parse" "--short" "HEAD"
        OUTPUT_VARIABLE NEST_VERSION_GITHASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    execute_process(
        COMMAND "git" "rev-parse" "--abbrev-ref" "HEAD"
        OUTPUT_VARIABLE NEST_VERSION_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    if (NEST_VERSION_SUFFIX)
        set(versionsuffix "-${NEST_VERSION_SUFFIX}")
    endif()

    if (NEST_VERSION_GITHASH)
        set(githash "@${NEST_VERSION_GITHASH}")
    endif()

    if (NOT NEST_VERSION_BRANCH)
        set(NEST_VERSION_BRANCH "UNKNOWN")
    endif()

    string(SUBSTRING "${NEST_VERSION_BRANCH}" 0 5 isRelease)
    if (isRelease STREQUAL "nest-")
        string(SUBSTRING "${NEST_VERSION_BRANCH}${versionsuffix}" 5 99999 NEST_VERSION)
    else()
        set(NEST_VERSION "${NEST_VERSION_BRANCH}${versionsuffix}")
    endif()

    set(NEST_VERSION_STRING "${NEST_VERSION_BRANCH}${versionsuffix}${githash}")
    unset(branchname)
    unset(versionsuffix)
    unset(githash)

endmacro()
