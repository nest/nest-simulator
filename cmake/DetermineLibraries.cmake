# cmake/DetermineLibraries.cmake
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

# Depending on the user options, we search for required libraries and include dirs.

# Only find libLTDL if we link dynamically
set( HAVE_LIBLTDL OFF )
if ( with-ltdl AND NOT static-libraries )
  if ( NOT ${with-ltdl} STREQUAL "ON" )
    # a path is set
    set( LTDL_ROOT_DIR ${with-ltdl} )
  endif ()

  find_package( LTDL )
  if ( LTDL_FOUND )
    set( HAVE_LIBLTDL ON )

    include_directories( ${LTDL_INCLUDE_DIRS} )
    # is linked in nestkernel/CMakeLists.txt
  endif ()
endif ()

# Find readline
set( HAVE_READLINE OFF )
if ( with-readline )
  if ( NOT ${with-readline} STREQUAL "ON" )
    # a path is set
    set( READLINE_ROOT_DIR ${with-readline} )
  endif ()

  find_package( Readline )
  if ( READLINE_FOUND )
    set( HAVE_READLINE ON )
    include_directories( ${READLINE_INCLUDE_DIRS} )
    # is linked in sli/CMakeLists.txt
  endif ()
endif ()

# Find GSL
set( HAVE_GSL OFF )
if ( with-gsl )
  if ( NOT ${with-gsl} STREQUAL "ON" )
    # if set, use this prefix
    set( GSL_ROOT_DIR ${with-gsl} )
  endif ()

  find_package( GSL )

  # only allow GSL 1.11 and later
  if ( GSL_FOUND AND ( "${GSL_VERSION}" VERSION_GREATER "1.11"
      OR "${GSL_VERSION}" VERSION_EQUAL "1.11" ))
    set( HAVE_GSL ON )
    include_directories( ${GSL_INCLUDE_DIRS} )
    # is linked in libnestutil/CMakeLists.txt
  endif ()
endif ()

# Find Python
set( HAVE_PYTHON OFF )
if ( with-python )
  if ( NOT ${with-python} STREQUAL "ON" )
    # a path is set
    set( PYTHON_EXECUTABLE ${with-python} )
  endif ()

  find_package( PythonInterp )
  if ( PYTHONINTERP_FOUND )
    set( PYTHON ${PYTHON_EXECUTABLE} )
    set( PYTHON_VERSION ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} )

    # need python lib and header...
    find_package( PythonLibs )
    if ( PYTHONLIBS_FOUND )
      set( HAVE_PYTHON ON )

      if ( cythonize-pynest )
        find_package( Cython )
        if ( CYTHON_FOUND )
          # confirmed not working: 0.15.1
          # confirmed working: 0.19.2+
          # in between unknown
          if ( CYTHON_VERSION VERSION_LESS "0.19.2" )
            message( FATAL_ERROR "Your Cython version is too old. Please install "
                                 "newer version (0.19.2+)" )
          endif ()
        endif ()
      endif ()

      set( PYEXECDIR "${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages" )
    endif ()
  endif ()
endif ()

# Find OPENMP
if ( with-openmp )
  if ( NOT ${with-openmp} STREQUAL "ON" )
    # a path is set
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${with-openmp}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${with-openmp}" )
  else ()
    find_package( OpenMP )
    if ( OPENMP_FOUND )
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
    endif ()
  endif ()
endif ()

# Find MPI
set( HAVE_MPI OFF )
if ( with-mpi )
  find_package( MPI )
  if ( MPI_CXX_FOUND )
    set( HAVE_MPI ON )

    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${MPI_C_COMPILE_FLAGS}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}" )

    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}" )
    include_directories( ${MPI_CXX_INCLUDE_PATH} )
    # is linked in nestkernel/CMakeLists.txt
  endif ()
endif ()

# Find libneurosim
set( HAVE_LIBNEUROSIM OFF )
if ( with-libneurosim )
  if ( NOT ${with-libneurosim} STREQUAL "ON" )
    # a path is set
    set( LIBNEUROSIM_ROOT ${with-libneurosim} )
  endif ()

  find_package( LibNeurosim )
  if ( LIBNEUROSIM_FOUND )
    set( HAVE_LIBNEUROSIM ON )

    include_directories( ${LIBNEUROSIM_INCLUDE_DIRS} )
    # is linked in conngen/CMakeLists.txt
  endif ()
endif ()

# Find music
set( HAVE_MUSIC OFF )
if ( with-music )
  if ( NOT ${with-music} STREQUAL "ON" )
    # a path is set
    set( MUSIC_ROOT_DIR "${with-music}" )
  endif ()

  if ( NOT HAVE_MPI )
    message( FATAL_ERROR "MUSIC requires -Dwith-mpi=ON." )
  endif ()

  find_package( Music )
  include_directories( ${MUSIC_INCLUDE_DIRS} )
  # is linked in nestkernel/CMakeLists.txt
  if ( MUSIC_FOUND )
    set( HAVE_MUSIC ON )
  endif ()
endif ()
