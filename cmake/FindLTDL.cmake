# FindLTDL.cmake
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

# - Find ltdl header and library
#
# This module defines
#  LTDL_FOUND, if false, do not try to use LTDL.
#  LTDL_INCLUDE_DIRS, where to find ltdl.h.
#  LTDL_LIBRARIES, the libraries to link against to use libltdl.
#  LTDL_VERSION, the library version
#  LIBTOOL_EXECUTABLE, executable that provide generalized library-building support services.
#
# As a hint allows LTDL_ROOT_DIR

find_path( LTDL_INCLUDE_DIR
    NAMES ltdl.h
    HINTS ${LTDL_ROOT_DIR}/include
    )
find_library( LTDL_LIBRARY
    NAMES ltdl
    HINTS ${LTDL_ROOT_DIR}/lib
    )
find_program( LIBTOOL_EXECUTABLE
    NAMES glibtool libtool libtoolize
    HINTS ${LTDL_ROOT_DIR}/bin
    )

if ( NOT LIBTOOL_EXECUTABLE STREQUAL "LIBTOOL_EXECUTABLE-NOTFOUND" )
  execute_process(
      COMMAND ${LIBTOOL_EXECUTABLE} --version
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE LTDL_VAR_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if ( RESULT EQUAL 0 )
    string( REGEX REPLACE ".* ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1"
                          LTDL_VERSION ${LTDL_VAR_OUTPUT} )
  endif ()
endif ()

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( LTDL
  FOUND_VAR
    LTDL_FOUND
  REQUIRED_VARS
    LTDL_LIBRARY
    LTDL_INCLUDE_DIR
  VERSION_VAR
    LTDL_VERSION
    )

if ( LTDL_FOUND )
  set( LTDL_INCLUDE_DIRS "${LTDL_INCLUDE_DIR}" )
  set( LTDL_LIBRARIES "${LTDL_LIBRARY}" )
endif ()

mark_as_advanced( LTDL_ROOT_DIR LTDL_INCLUDE_DIR LTDL_LIBRARY LIBTOOL_EXECUTABLE )
