# FindSIONlib.cmake
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

# - Find SIONlib header and library
#
# This module defines
#  SIONLIB_FOUND, if false, do not try to use SIONlib.
#  SIONLIB_INCLUDE, where to find sion.h.
#  SIONLIB_LIBRARIES, the libraries to link against to use SIONlib.
#  SIONLIB_DEFINES, the defines needed to link against SIONlib.
#  SIONLIB_CONFIG, the sionconfig executable.
#  SIONLIB_VERSION, the library version
#
# As a hint allows SIONLIB_ROOT_DIR.

# find include dir
find_path( SIONLIB_INCLUDE
    NAMES sion.h
    HINTS ${SIONLIB_ROOT_DIR}/include
    )

# find sionconfig executable
find_program( SIONLIB_CONFIG
    NAMES sionconfig
    HINTS ${SIONLIB_ROOT_DIR}/bin
    )

if ( NOT SIONLIB_CONFIG STREQUAL "SIONLIB_CONFIG-NOTFOUND" )
  # use sionconfig to get version
  execute_process(
      COMMAND ${SIONLIB_CONFIG} --version
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE SIONLIB_VAR_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if ( RESULT EQUAL 0 )
    string( REGEX REPLACE "^SIONlib Version ([0-9]\\.[0-9]\\.[0-9]).*" "\\1"
                          SIONLIB_VERSION ${SIONLIB_VAR_OUTPUT} )
  endif ()

  # get arguments for sionconfig --cflags and --libs
  set( CONF_FLAGS "--cxx" ) # we use cxx
  # find parallelization
  if ( OPENMP_FOUND AND MPI_CXX_FOUND )
    set( CONF_FLAGS ${CONF_FLAGS} "--ompi" )
  elseif ( OPENMP_FOUND )
    set( CONF_FLAGS ${CONF_FLAGS} "--omp" )
  elseif ( MPI_CXX_FOUND )
    set( CONF_FLAGS ${CONF_FLAGS} "--mpi" )
  endif ()

  # what compiler?
  if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    set( CONF_FLAGS ${CONF_FLAGS} "--gcc" )
  endif ()


  # use sionconfig to get --cflags
  execute_process(
      COMMAND ${SIONLIB_CONFIG} ${CONF_FLAGS} --cflags
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE SIONLIB_CFLAGS_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  if ( RESULT EQUAL 0 )
    # only extract defines; include is already looked for before
    set( SIONLIB_DEFINES "" CACHE INTERNAL "cmake sucks" )
    # make list
    string( REPLACE " " ";" FLAGS ${SIONLIB_CFLAGS_OUTPUT} )
    foreach ( def ${FLAGS} )
      # def starts with -D....
      if ( "${def}" MATCHES "^-D.*" )
        # add to defines
        set( SIONLIB_DEFINES ${SIONLIB_DEFINES} "${def}" CACHE INTERNAL "cmake sucks" )
        # add current definition at CMake level
        add_definitions( "${def}" )
      endif ()
    endforeach ()
  endif ()

  # use sionconfig to get --libs
  execute_process(
      COMMAND ${SIONLIB_CONFIG} ${CONF_FLAGS} --libs
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE SIONLIB_LIBS_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  if ( RESULT EQUAL 0 )
    set( SIONLIB_LIBRARIES "" CACHE INTERNAL "cmake sucks" )
    # make list
    string( REPLACE " " ";" LIBS ${SIONLIB_LIBS_OUTPUT} )
    foreach ( lib ${LIBS} )
      # is a -l... library
      if ( "${lib}" MATCHES "^-l.*" )
        # only get lib name
        string( REGEX REPLACE "^-l(.*)" "\\1" lib_name ${lib} )
        # use a separate variable for every library, otherwise, if already set,
        # it will be ignored as has found
        find_library( FULL_${lib_name}
            NAMES ${lib_name}
            HINTS ${SIONLIB_ROOT_DIR}/lib
            )
        if ( NOT FULL_${lib_name} STREQUAL "FULL_${lib_name}-NOTFOUND" )
          set( SIONLIB_LIBRARIES ${SIONLIB_LIBRARIES} ${FULL_${lib_name}} CACHE INTERNAL "cmake sucks" )
        else ()
          message( WARNING "Cannot find SIONlib library '${lib_name}'." )
          unset( SIONLIB_LIBRARIES CACHE )
          break()
        endif ()
      endif ()
    endforeach ()
  endif ()
endif ()

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( SIONlib
  FOUND_VAR
    SIONLIB_FOUND
  REQUIRED_VARS
    SIONLIB_LIBRARIES
    SIONLIB_INCLUDE
    SIONLIB_DEFINES
    SIONLIB_CONFIG
  VERSION_VAR
    SIONLIB_VERSION
    )

mark_as_advanced( SIONLIB_ROOT_DIR SIONLIB_INCLUDE SIONLIB_DEFINES
                  SIONLIB_LIBRARIES SIONLIB_CONFIG )
