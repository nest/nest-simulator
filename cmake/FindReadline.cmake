# FindReadline.cmake
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

# - Find GNU Readline header and library
#
# This module defines
#  READLINE_FOUND, if false, do not try to use GNU Readline.
#  READLINE_INCLUDE_DIRS, where to find readline/readline.h.
#  READLINE_LIBRARIES, the libraries to link against to use GNU Readline.
#  READLINE_VERSION, the library version
#
# As a hint allows READLINE_ROOT_DIR


find_path( READLINE_INCLUDE_DIR
    NAMES readline/readline.h
    HINTS ${READLINE_ROOT_DIR}/include
    NO_SYSTEM_ENVIRONMENT_PATH  # anaconda python tries to be first in path and hides a useful readline
    )
find_library( READLINE_LIBRARY
    NAMES readline
    HINTS ${READLINE_ROOT_DIR}/lib
    NO_SYSTEM_ENVIRONMENT_PATH  # anaconda python tries to be first in path and hides a useful readline
    )
find_library( NCURSES_LIBRARY       # readline depends on libncurses, or similar
    NAMES ncurses ncursesw curses termcap
    HINTS ${READLINE_ROOT_DIR}/lib
    )

if ( EXISTS "${READLINE_INCLUDE_DIR}/readline/readline.h" )
  file( STRINGS "${READLINE_INCLUDE_DIR}/readline/readline.h" readline_h_content
                REGEX "#define RL_READLINE_VERSION" )
  string( REGEX REPLACE ".*0x([0-9][0-9])([0-9][0-9]).*" "\\1.\\2"
                        READLINE_VERSION ${readline_h_content} )
  string( REGEX REPLACE "^0" "" READLINE_VERSION ${READLINE_VERSION} )
  string( REPLACE ".0" "." READLINE_VERSION ${READLINE_VERSION} )
endif ()

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Readline
  FOUND_VAR
    READLINE_FOUND
  REQUIRED_VARS
    READLINE_LIBRARY
    NCURSES_LIBRARY
    READLINE_INCLUDE_DIR
  VERSION_VAR
    READLINE_VERSION
    )

if ( READLINE_FOUND )
  set( READLINE_LIBRARIES "${READLINE_LIBRARY}" "${NCURSES_LIBRARY}" )
  set( READLINE_INCLUDE_DIRS "${READLINE_INCLUDE_DIR}" )
endif ()

mark_as_advanced( READLINE_ROOT_DIR READLINE_INCLUDE_DIR READLINE_LIBRARY NCURSES_LIBRARY )
