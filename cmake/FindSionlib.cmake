# FindSionlib.cmake
#
# This file is part of NEST.
#
# Copyright (C) 2016 The NEST Initiative
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

# - Find MUSIC header and library
#
# This module defines
#  MUSIC_FOUND, if false, do not try to use MUSIC.
#  MUSIC_INCLUDE_DIRS, where to find music.hh.
#  MUSIC_LIBRARIES, the libraries to link against to use MUSIC.
#  MUSIC_EXECUTABLE, the music executable.
#  MUSIC_VERSION, the library version
#
# As a hint allows MUSIC_ROOT_DIR.

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(SION_GCC_APPEND '--gcc')
else()
  set(SION_GCC_APPEND '')
endif()

if ( "${with-sionlib}" STREQUAL "ON" )
  set(SION_EXECUTABLE "sionconfig")
else()
  set(SION_EXECUTABLE "${with-sionlib}/sionconfig")
endif()

execute_process(
  COMMAND ${SION_EXECUTABLE} --cflags ${SION_GCC_APPEND}
  OUTPUT_VARIABLE SION_INCLUDE
  )
execute_process(
  COMMAND ${SION_EXECUTABLE} --ompi --libs ${SION_GCC_APPEND}
  OUTPUT_VARIABLE SION_LIBS
  )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Sionlib
  FOUND_VAR
    SIONLIB_FOUND
  REQUIRED_VARS
    SION_INCLUDE
    SION_LIBS
    )

mark_as_advanced( SION_GCC_APPEND SION_EXECUTABLE SION_INCLUDE SION_LIBS )
