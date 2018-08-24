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
function( NEST_PROCESS_WITH_OPTIMIZE )
  if ( with-optimize )
    if ( with-optimize STREQUAL "ON" )
      set( with-optimize "-O2" )
    endif ()
    foreach ( flag ${with-optimize} )
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE )
    endforeach ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_DEBUG )
  if ( with-debug )
    if ( with-debug STREQUAL "ON" )
      set( with-debug "-g" )
    endif ()
    foreach ( flag ${with-debug} )
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE )
    endforeach ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_WARNING )
  if ( with-warning )
    if ( with-warning STREQUAL "ON" )
      if ( NOT k-computer STREQUAL "ON" )
        set( with-warning "-Wall" )
      else()
        set( with-warning "" )
      endif()
    endif ()
    foreach ( flag ${with-warning} )
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE )
    endforeach ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_LIBRARIES )
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
endfunction()

function( NEST_PROCESS_WITH_INCLUDES )
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
endfunction()

function( NEST_PROCESS_WITH_DEFINES )
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
endfunction()

function( NEST_PROCESS_K_COMPUTER )
  # is set in the Fujitsu-Sparc64.cmake file
  if ( k-computer )
    set( IS_K ON PARENT_SCOPE )
    # need alternative tokens command to compile NEST
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --alternative_tokens" PARENT_SCOPE )
    # FCC accepts GNU flags when -Xg is supplied
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xg --alternative_tokens" PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_ENABLE_BLUEGENE )
  # is set in the BlueGeneQ.cmake file
  if ( enable-bluegene )
    if ( ${enable-bluegene} STREQUAL "L" )
      set( IS_BLUEGENE_L ON PARENT_SCOPE )
    elseif ( ${enable-bluegene} STREQUAL "P" )
      set( IS_BLUEGENE_P ON PARENT_SCOPE )
    elseif ( ${enable-bluegene} STREQUAL "Q" )
      set( IS_BLUEGENE_Q ON PARENT_SCOPE )
    else ()
      message( FATAL_ERROR "Only L/P/Q is allowed for enable-bluegene." )
    endif ()
    set( IS_BLUEGENE ON PARENT_SCOPE )
  else ()
    set( IS_BLUEGENE OFF PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_GET_COLOR_FLAGS )
    set( NEST_C_COLOR_FLAGS "" PARENT_SCOPE )
    set( NEST_CXX_COLOR_FLAGS "" PARENT_SCOPE )

    # add colored output from gcc
    if ( CMAKE_C_COMPILER_ID STREQUAL "GNU" )
      if ( NOT CMAKE_C_COMPILER_VERSION VERSION_LESS "4.9" )
        set( NEST_C_COLOR_FLAGS "-fdiagnostics-color=auto" PARENT_SCOPE )
      endif ()
    endif ()
    if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
      if ( NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9" )
        set( NEST_CXX_COLOR_FLAGS "-fdiagnostics-color=auto" PARENT_SCOPE )
      endif ()
    endif ()
endfunction()

function( NEST_PROCESS_STATIC_LIBRARIES )
  # build static or shared libraries
  if ( static-libraries )
    set( BUILD_SHARED_LIBS OFF PARENT_SCOPE )
    # set RPATH stuff
    set( CMAKE_SKIP_RPATH TRUE PARENT_SCOPE )

    if ( UNIX OR APPLE )
      # On Linux .a is the static library suffix, on Mac OS X .lib can also
      # be used, so we'll add both to the preference list.
      set( CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib;.dylib;.so" PARENT_SCOPE )
    endif ()
  else ()
    set( BUILD_SHARED_LIBS ON PARENT_SCOPE )

    # set RPATH stuff
    set( CMAKE_SKIP_RPATH FALSE PARENT_SCOPE )
    # use, i.e. don't skip the full RPATH for the build tree
    set( CMAKE_SKIP_BUILD_RPATH FALSE PARENT_SCOPE )
    # on OS X
    set( CMAKE_MACOSX_RPATH ON PARENT_SCOPE )

    # when building, don't use the install RPATH already
    # (but later on when installing)
    set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE PARENT_SCOPE )

    # set the rpath only when installed
    if ( APPLE )
      set( CMAKE_INSTALL_RPATH
          "@loader_path/../${CMAKE_INSTALL_LIBDIR}"
          "@loader_path/../${CMAKE_INSTALL_LIBDIR}/nest"
          # for pynestkernel: @loader_path at <prefix>/lib/python2.7/site-packages/nest
          "@loader_path/../../.."
          "@loader_path/../../../nest"
          PARENT_SCOPE )
    else ()
      set( CMAKE_INSTALL_RPATH
          "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
          "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/nest"
          # for pynestkernel: origin at <prefix>/lib/python2.7/site-packages/nest
          "\$ORIGIN/../../.."
          "\$ORIGIN/../../../nest"
          PARENT_SCOPE )
    endif ()

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE PARENT_SCOPE )

    if ( UNIX OR APPLE )
      # reverse the search order for lib extensions
      set( CMAKE_FIND_LIBRARY_SUFFIXES ".so;.dylib;.a;.lib" PARENT_SCOPE )
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_EXTERNAL_MODULES )
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
          HINTS "${CMAKE_INSTALL_FULL_INCLUDEDIR}/${mod}module"
          )
      if ( ${mod}_EXT_MOD_INCLUDE STREQUAL "${mod}_EXT_MOD_INCLUDE-NOTFOUND" )
        message( FATAL_ERROR "Cannot find header for external module '${mod}'. "
          "Should be '${CMAKE_INSTALL_FULL_INCLUDEDIR}/${mod}module/${mod}module.h' ." )
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

    set( EXTERNAL_MODULE_LIBRARIES ${EXTERNAL_MODULE_LIBRARIES} PARENT_SCOPE )
    set( EXTERNAL_MODULE_INCLUDES ${EXTERNAL_MODULE_INCLUDES} PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_TICS_PER_MS )
  # Set tics per ms / step
  if ( tics_per_ms )
    set( HAVE_TICS_PER_MS ON PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_TICS_PER_STEP )
  if ( tics_per_step )
    set( HAVE_TICS_PER_STEP ON PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_PS_ARRAY )
  if ( with-ps-arrays )
    set( PS_ARRAYS ON PARENT_SCOPE )
  endif ()
endfunction()

# Depending on the user options, we search for required libraries and include dirs.

function( NEST_PROCESS_WITH_LIBLTDL )
  # Only find libLTDL if we link dynamically
  set( HAVE_LIBLTDL OFF PARENT_SCOPE )
  if ( with-ltdl AND NOT static-libraries )
    if ( NOT ${with-ltdl} STREQUAL "ON" )
      # a path is set
      set( LTDL_ROOT_DIR "${with-ltdl}" )
    endif ()

    find_package( LTDL )
    if ( LTDL_FOUND )
      set( HAVE_LIBLTDL ON PARENT_SCOPE )
      # export found variables to parent scope
      set( LTDL_FOUND ON PARENT_SCOPE )
      set( LTDL_LIBRARIES "${LTDL_LIBRARIES}" PARENT_SCOPE )
      set( LTDL_INCLUDE_DIRS "${LTDL_INCLUDE_DIRS}" PARENT_SCOPE )
      set( LTDL_VERSION "${LTDL_VERSION}" PARENT_SCOPE )

      include_directories( ${LTDL_INCLUDE_DIRS} )
      # is linked in nestkernel/CMakeLists.txt
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_READLINE )
  # Find readline
  set( HAVE_READLINE OFF PARENT_SCOPE )
  if ( with-readline )
    if ( NOT ${with-readline} STREQUAL "ON" )
      # a path is set
      set( READLINE_ROOT_DIR "${with-readline}" )
    endif ()

    find_package( Readline )
    if ( READLINE_FOUND )
      set( HAVE_READLINE ON PARENT_SCOPE )
      # export found variables to parent scope
      set( READLINE_FOUND "${READLINE_FOUND}" PARENT_SCOPE )
      set( READLINE_LIBRARIES "${READLINE_LIBRARIES}" PARENT_SCOPE )
      set( READLINE_INCLUDE_DIRS "${READLINE_INCLUDE_DIRS}" PARENT_SCOPE )
      set( READLINE_VERSION "${READLINE_VERSION}" PARENT_SCOPE )

      include_directories( ${READLINE_INCLUDE_DIRS} )
      # is linked in sli/CMakeLists.txt
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_GSL )
  # Find GSL
  set( HAVE_GSL OFF PARENT_SCOPE )
  if ( with-gsl )
    if ( NOT ${with-gsl} STREQUAL "ON" )
      # if set, use this prefix
      set( GSL_ROOT_DIR "${with-gsl}" )
    endif ()

    find_package( GSL )

    # only allow GSL 1.11 and later
    if ( GSL_FOUND AND ( "${GSL_VERSION}" VERSION_GREATER "1.11"
        OR "${GSL_VERSION}" VERSION_EQUAL "1.11" ))
      set( HAVE_GSL ON PARENT_SCOPE )

      # export found variables to parent scope
      set( GSL_VERSION "${GSL_VERSION}" PARENT_SCOPE )
      set( GSL_LIBRARIES "${GSL_LIBRARIES}" PARENT_SCOPE )
      set( GSL_INCLUDE_DIRS "${GSL_INCLUDE_DIRS}" PARENT_SCOPE )

      include_directories( ${GSL_INCLUDE_DIRS} )
      # is linked in libnestutil/CMakeLists.txt
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_PYTHON )
  # Find Python
  set( HAVE_PYTHON OFF PARENT_SCOPE )
  if ( ${with-python} STREQUAL "ON" OR  ${with-python} STREQUAL "2" OR  ${with-python} STREQUAL "3" )

    # Localize the Python interpreter
    if ( ${with-python} STREQUAL "ON" )
      find_package( PythonInterp )
    elseif ( ${with-python} STREQUAL "2" )  
      find_package( PythonInterp 2 REQUIRED )
    elseif ( ${with-python} STREQUAL "3" )
      find_package( PythonInterp 3 REQUIRED )
    endif ()

    if ( PYTHONINTERP_FOUND )
      set( PYTHONINTERP_FOUND "${PYTHONINTERP_FOUND}" PARENT_SCOPE )
      set( PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE} PARENT_SCOPE )
      set( PYTHON ${PYTHON_EXECUTABLE} PARENT_SCOPE )
      set( PYTHON_VERSION ${PYTHON_VERSION_STRING} PARENT_SCOPE )

      # Localize Python lib/header files and make sure that their version matches 
      # the Python interpreter version !
      find_package( PythonLibs ${PYTHON_VERSION_STRING} EXACT )
      if ( PYTHONLIBS_FOUND )
        set( HAVE_PYTHON ON PARENT_SCOPE )
        # export found variables to parent scope
        set( PYTHONLIBS_FOUND "${PYTHONLIBS_FOUND}" PARENT_SCOPE )
        set( PYTHON_INCLUDE_DIRS "${PYTHON_INCLUDE_DIRS}" PARENT_SCOPE )
        set( PYTHON_LIBRARIES "${PYTHON_LIBRARIES}" PARENT_SCOPE )

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

            # export found variables to parent scope
            set( CYTHON_FOUND "${CYTHON_FOUND}" PARENT_SCOPE )
            set( CYTHON_EXECUTABLE "${CYTHON_EXECUTABLE}" PARENT_SCOPE )
            set( CYTHON_VERSION "${CYTHON_VERSION}" PARENT_SCOPE )
          endif ()
        endif ()
        set( PYEXECDIR "${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages" PARENT_SCOPE )
      endif ()
    endif ()
  elseif ( ${with-python} STREQUAL "OFF" )
  else ()
    message( FATAL_ERROR "Invalid option: -Dwith-python=" ${with-python} )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_OPENMP )
  # Find OPENMP
  if ( with-openmp )
    if ( NOT "${with-openmp}" STREQUAL "ON" )
      message( STATUS "Set OpenMP argument: ${with-openmp}")
      # set variablesin this scope
      set( OPENMP_FOUND ON )
      set( OpenMP_C_FLAGS "${with-openmp}" )
      set( OpenMP_CXX_FLAGS "${with-openmp}" )
    else ()
      find_package( OpenMP )
    endif ()
    if ( OPENMP_FOUND )
      # export found variables to parent scope
      set( OPENMP_FOUND "${OPENMP_FOUND}" PARENT_SCOPE )
      set( OpenMP_C_FLAGS "${OpenMP_C_FLAGS}" PARENT_SCOPE )
      set( OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS}" PARENT_SCOPE )
      # set flags
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" PARENT_SCOPE )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" PARENT_SCOPE )
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_MPI )
  # Find MPI
  set( HAVE_MPI OFF PARENT_SCOPE )
  if ( with-mpi )
    find_package( MPI )
    if ( MPI_CXX_FOUND )
      set( HAVE_MPI ON PARENT_SCOPE )

      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${MPI_C_COMPILE_FLAGS}" PARENT_SCOPE )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}" PARENT_SCOPE )

      set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}" PARENT_SCOPE )
      include_directories( ${MPI_CXX_INCLUDE_PATH} )
      # is linked in nestkernel/CMakeLists.txt

      # export found variables to parent scope
      set( MPI_C_FOUND "${MPI_C_FOUND}" PARENT_SCOPE )
      set( MPI_C_COMPILER "${MPI_C_COMPILER}" PARENT_SCOPE )
      set( MPI_C_COMPILE_FLAGS "${MPI_C_COMPILE_FLAGS}" PARENT_SCOPE )
      set( MPI_C_INCLUDE_PATH "${MPI_C_INCLUDE_PATH}" PARENT_SCOPE )
      set( MPI_C_LINK_FLAGS "${MPI_C_LINK_FLAGS}" PARENT_SCOPE )
      set( MPI_C_LIBRARIES "${MPI_C_LIBRARIES}" PARENT_SCOPE )
      set( MPI_CXX_FOUND "${MPI_CXX_FOUND}" PARENT_SCOPE )
      set( MPI_CXX_COMPILER "${MPI_CXX_COMPILER}" PARENT_SCOPE )
      set( MPI_CXX_COMPILE_FLAGS "${MPI_CXX_COMPILE_FLAGS}" PARENT_SCOPE )
      set( MPI_CXX_INCLUDE_PATH "${MPI_CXX_INCLUDE_PATH}" PARENT_SCOPE )
      set( MPI_CXX_LINK_FLAGS "${MPI_CXX_LINK_FLAGS}" PARENT_SCOPE )
      set( MPI_CXX_LIBRARIES "${MPI_CXX_LIBRARIES}" PARENT_SCOPE )
      set( MPIEXEC "${MPIEXEC}" PARENT_SCOPE )
      set( MPIEXEC_NUMPROC_FLAG "${MPIEXEC_NUMPROC_FLAG}" PARENT_SCOPE )
      set( MPIEXEC_PREFLAGS "${MPIEXEC_PREFLAGS}" PARENT_SCOPE )
      set( MPIEXEC_POSTFLAGS "${MPIEXEC_POSTFLAGS}" PARENT_SCOPE )
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_LIBNEUROSIM )
  # Find libneurosim
  set( HAVE_LIBNEUROSIM OFF PARENT_SCOPE )
  if ( with-libneurosim )
    if ( NOT ${with-libneurosim} STREQUAL "ON" )
      # a path is set
      set( LIBNEUROSIM_ROOT ${with-libneurosim} )
    endif ()

    find_package( LibNeurosim )
    if ( LIBNEUROSIM_FOUND )
      set( HAVE_LIBNEUROSIM ON PARENT_SCOPE )

      include_directories( ${LIBNEUROSIM_INCLUDE_DIRS} )
      # is linked in conngen/CMakeLists.txt

      # export found variables to parent scope
      set( LIBNEUROSIM_FOUND "${LIBNEUROSIM_FOUND}" PARENT_SCOPE )
      set( LIBNEUROSIM_LIBRARIES "${LIBNEUROSIM_LIBRARIES}" PARENT_SCOPE )
      set( LIBNEUROSIM_INCLUDE_DIRS "${LIBNEUROSIM_INCLUDE_DIRS}" PARENT_SCOPE )
      set( LIBNEUROSIM_VERSION "${LIBNEUROSIM_VERSION}" PARENT_SCOPE )
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_MUSIC )
  # Find music
  set( HAVE_MUSIC OFF PARENT_SCOPE )
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
      # export found variables to parent scope
      set( HAVE_MUSIC ON PARENT_SCOPE )
      set( MUSIC_FOUND "${MUSIC_FOUND}" PARENT_SCOPE )
      set( MUSIC_LIBRARIES "${MUSIC_LIBRARIES}" PARENT_SCOPE )
      set( MUSIC_INCLUDE_DIRS "${MUSIC_INCLUDE_DIRS}" PARENT_SCOPE )
      set( MUSIC_EXECUTABLE "${MUSIC_EXECUTABLE}" PARENT_SCOPE )
      set( MUSIC_VERSION "${MUSIC_VERSION}" PARENT_SCOPE )
    endif ()
  endif ()
endfunction()

function( NEST_PROCESS_WITH_BOOST )
  # Find Boost
  set( HAVE_BOOST OFF PARENT_SCOPE )
  if ( with-boost )
    if ( NOT ${with-boost} STREQUAL "ON" )
      # a path is set
      set( BOOST_ROOT "${with-boost}" )
    endif ()

    find_package( Boost COMPONENTS unit_test_framework )
    if ( Boost_FOUND )
      # export found variables to parent scope
      set( HAVE_BOOST ON PARENT_SCOPE )
      # Boost uses lower case in variable names
      set( BOOST_FOUND "${Boost_FOUND}" PARENT_SCOPE )
      set( BOOST_LIBRARIES "${Boost_LIBRARIES}" PARENT_SCOPE )
      set( BOOST_INCLUDE_DIR "${Boost_INCLUDE_DIR}" PARENT_SCOPE )
      set( BOOST_VERSION "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}" PARENT_SCOPE )
    endif ()
  endif ()
endfunction()

function( NEST_DEFAULT_MODULES )
    # requires HAVE_LIBNEUROSIM set
    # Static modules
    set( SLI_MODULES models precise topology )
    if ( HAVE_LIBNEUROSIM )
      set( SLI_MODULES ${SLI_MODULES} conngen )
    endif ()
    set( SLI_MODULES ${SLI_MODULES} PARENT_SCOPE )

    set( SLI_MODULE_INCLUDE_DIRS )
    foreach ( mod ${SLI_MODULES} )
      list( APPEND SLI_MODULE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/${mod}" )
    endforeach ()
    set( SLI_MODULE_INCLUDE_DIRS ${SLI_MODULE_INCLUDE_DIRS} PARENT_SCOPE )
endfunction()
