# cmake/ProcessOptions.cmake
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

# Here all user defined options will be processed.

# add custom warnings and optimizations
if ( with-optimize )
  if ( with-optimize STREQUAL "ON" )
    set( with-optimize "-O2" )
  endif ()
  foreach ( flag ${with-optimize} )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" )
  endforeach ()
endif ()

if ( with-debug )
  if ( with-debug STREQUAL "ON" )
    set( with-debug "-g" )
  endif ()
  foreach ( flag ${with-debug} )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" )
  endforeach ()
endif ()

if ( with-warning )
  if ( with-warning STREQUAL "ON" )
    set( with-warning "-Wall" )
  endif ()
  foreach ( flag ${with-warning} )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" )
  endforeach ()
endif ()

if ( with-libraries )
  if ( with-libraries STREQUAL "ON" )
    message( FATAL_ERROR "-Dwith-libraries requires full library paths." )
  endif ()
  foreach ( lib ${with-libraries} )
    if ( EXISTS "${lib}" )
      link_libraries( "${lib}" )
    else ()
      message( FATAL_ERROR "Library '${lib}' does not exist!" )
    endif ()
  endforeach ()
endif ()

if ( with-includes )
  if ( with-includes STREQUAL "ON" )
    message( FATAL_ERROR "-Dwith-includes requires full paths." )
  endif ()
  foreach ( inc ${with-includes} )
    if ( IS_DIRECTORY "${inc}" )
      include_directories( "${inc}" )
    else ()
      message( FATAL_ERROR "Include path '${inc}' does not exist!" )
    endif ()
  endforeach ()
endif ()

if ( with-defines )
  if ( with-defines STREQUAL "ON" )
    message( FATAL_ERROR "-Dwith-defines requires compiler defines -DXYZ=... ." )
  endif ()
  foreach ( def ${with-defines} )
    if ( "${def}" MATCHES "^-D.*" )
      add_definitions( "${def}" )
    else ()
      message( FATAL_ERROR "Define '${def}' does not match '-D.*' !" )
    endif ()
  endforeach ()
endif ()

# add colored output from gcc
if ( CMAKE_C_COMPILER_ID STREQUAL "GNU" )
  if ( NOT CMAKE_C_COMPILER_VERSION VERSION_LESS "4.9" )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=auto" )
  endif ()
endif ()
if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  if ( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto" )
  endif ()
endif ()

# is set in the Fujitsu-Sparc64.cmake file
if ( k-computer )
  set( IS_K ON )
  # need alternative tokens command to compile NEST
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --alternative_tokens" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --alternative_tokens" )
endif ()

# is set in the BlueGeneQ.cmake file
if ( enable-bluegene )
  if ( ${enable-bluegene} STREQUAL "L" )
    set( IS_BLUEGENE_L ON )
  elseif ( ${enable-bluegene} STREQUAL "P" )
    set( IS_BLUEGENE_P ON )
  elseif ( ${enable-bluegene} STREQUAL "Q" )
    set( IS_BLUEGENE_Q ON )
  else ()
    message( FATAL_ERROR "Only L/P/Q is allowed for enable-bluegene." )
  endif ()
  set( IS_BLUEGENE ON )
else ()
  set( IS_BLUEGENE OFF )
endif ()

# build static or shared libraries
if ( static-libraries )
  set( BUILD_SHARED_LIBS OFF )
  # set RPATH stuff
  set( CMAKE_SKIP_RPATH TRUE )

  if ( UNIX OR APPLE )
    # On Linux .a is the static library suffix, on Mac OS X .lib can also
    # be used, so we'll add both to the preference list.
    set( CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib;.dylib;.so" )
  endif ()
else ()
  set( BUILD_SHARED_LIBS ON )

  # set RPATH stuff
  set( CMAKE_SKIP_RPATH FALSE )
  # use, i.e. don't skip the full RPATH for the build tree
  set( CMAKE_SKIP_BUILD_RPATH FALSE )
  # on OS X
  set( CMAKE_MACOSX_RPATH ON )

  # when building, don't use the install RPATH already
  # (but later on when installing)
  set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )

  # set the rpath only when installed
  if ( APPLE )
    set( CMAKE_INSTALL_RPATH
        "@loader_path/../${CMAKE_INSTALL_LIBDIR}"
        "@loader_path/../${CMAKE_INSTALL_LIBDIR}/nest"
        # for pynestkernel: @loader_path at <prefix>/lib/python2.7/site-packages/nest
        "@loader_path/../../.."
        "@loader_path/../../../nest"
       )
  else ()
    set( CMAKE_INSTALL_RPATH
        "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
        "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/nest"
        # for pynestkernel: origin at <prefix>/lib/python2.7/site-packages/nest
        "\$ORIGIN/../../.."
        "\$ORIGIN/../../../nest"
        )
  endif ()

  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )

  if ( UNIX OR APPLE )
    # reverse the search order for lib extensions
    set( CMAKE_FIND_LIBRARY_SUFFIXES ".so;.dylib;.a;.lib" )
  endif ()
endif ()

# Static modules
set( SLI_MODULES models precise topology )
if ( HAVE_LIBNEUROSIM )
  set( SLI_MODULES ${SLI_MODULES} conngen )
endif ()

set( SLI_MODULE_INCLUDE_DIRS )
foreach ( mod ${SLI_MODULES} )
  list( APPEND SLI_MODULE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/${mod}" )
endforeach ()

if ( external-modules )
  # headers from external modules will be installed here
  include_directories( "${CMAKE_INSTALL_FULL_INCLUDEDIR}" )

  # put all external libs into this variable
  set( EXTERNAL_MODULE_LIBRARIES )
  # put all external headers into this variable
  set( EXTERNAL_MODULE_INCLUDES )
  foreach ( mod ${external-modules} )
    # find module header
    find_file( ${mod}_EXT_MOD_INCLUDE
        NAMES ${mod}module.h
        HINTS "${CMAKE_INSTALL_FULL_INCLUDEDIR}"
        )
    if ( ${mod}_EXT_MOD_INCLUDE STREQUAL "${mod}_EXT_MOD_INCLUDE-NOTFOUND" )
      message( FATAL_ERROR "Cannot find header for external module '${mod}'. "
        "Should be '${CMAKE_INSTALL_FULL_INCLUDEDIR}/${mod}module.h' ." )
    endif ()
    list( APPEND EXTERNAL_MODULE_INCLUDES ${${mod}_EXT_MOD_INCLUDE} )

    # find module library
    find_library( ${mod}_EXT_MOD_LIBRARY
        NAMES ${mod}module
        HINTS "${CMAKE_INSTALL_FULL_LIBDIR}/nest"
        )
    if ( ${mod}_EXT_MOD_LIBRARY STREQUAL "${mod}_EXT_MOD_LIBRARY-NOTFOUND" )
      message( FATAL_ERROR "Cannot find library for external module '${mod}'." )
    endif ()
    list( APPEND EXTERNAL_MODULE_LIBRARIES "${${mod}_EXT_MOD_LIBRARY}" )
  endforeach ()
endif ()

# Set tics per ms / step
if ( tics_per_ms )
  set( HAVE_TICS_PER_MS ON )
endif ()

if ( tics_per_step )
  set( HAVE_TICS_PER_STEP ON )
endif ()

if ( with-ps-arrays )
  set( PS_ARRAYS ON )
endif ()
