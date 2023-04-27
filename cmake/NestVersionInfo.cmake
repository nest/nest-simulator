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

# Put NEST version number and version control information into CMake variables
#
# This module defines
#   NEST_VERSION, the numeric version number including a suffix
#   NEST_VERSION_GIT, a boolean indicating if source is managed by git
#   NEST_VERSION_GIT_HASH, the current git revision hash (unset for tarballs)
#   NEST_VERSION_GIT_BRANCH, the current git branch (unset for tarballs)
#   NEST_VERSION_GIT_REMOTE, the upstream remote (if any) of the tracked branch (unset for tarballs)
#

macro(get_version_info)

  file (STRINGS "${CMAKE_SOURCE_DIR}/VERSION" NEST_VERSION )

  if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set( NEST_VERSION_GIT 1 )
    execute_process(
      COMMAND "bash" "${PROJECT_SOURCE_DIR}/build_support/version_info.sh"
      OUTPUT_VARIABLE NEST_VERSION_INFO
      OUTPUT_STRIP_TRAILING_WHITESPACE
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    list( GET NEST_VERSION_INFO 0 NEST_VERSION_GIT_HASH )
    list( GET NEST_VERSION_INFO 1 NEST_VERSION_GIT_BRANCH )
    list( GET NEST_VERSION_INFO 2 NEST_VERSION_GIT_REMOTE )
  endif()

endmacro()
