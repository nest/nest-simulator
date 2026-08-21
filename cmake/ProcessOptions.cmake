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

################################################################################
# Helper functions
################################################################################

# Regular expressions for CMake's false-ish and true-ish boolean spellings.
# Defined once here and inherited by all helper functions below.
set( _NEST_FALSE_REGEX "^(0|OFF|NO|FALSE|N|IGNORE|NOTFOUND)?$" )
set( _NEST_TRUE_REGEX  "^(1|ON|YES|TRUE|Y)$" )

# Validate that option_value is a recognised CMake boolean spelling
# (ON/YES/TRUE/Y/1 or OFF/NO/FALSE/N/IGNORE/NOTFOUND/empty); terminate with a
# clear error otherwise. Sets result_var to ON or OFF in the caller's scope.
# Validation uses a regex; normalisation delegates to CMake's own if() evaluator
# so that any future CMake boolean spellings are handled correctly automatically.
function( NEST_VALIDATE_BOOL_OPTION option_name option_value result_var )
  string( TOUPPER "${option_value}" _upper )
  if ( NOT ( _upper MATCHES "${_NEST_TRUE_REGEX}"
          OR _upper MATCHES "${_NEST_FALSE_REGEX}" ) )
    message( FATAL_ERROR
      "Invalid value -D${option_name}=${option_value}, please use 'ON' or 'OFF'." )
  endif ()
  # CMake's own if() is the canonical boolean evaluator; use it for normalisation.
  if ( "${option_value}" )
    set( ${result_var} ON PARENT_SCOPE )
  else ()
    set( ${result_var} OFF PARENT_SCOPE )
  endif ()
endfunction()

# Claim <package_name>_ROOT as a typed CACHE PATH variable and, when the user
# has pinned a location via that variable (or its ENV counterpart, or via
# CMAKE_PREFIX_PATH), restrict the subsequent find_package() to that path only
# by setting in the caller's (processor function's) scope:
#   CMAKE_PREFIX_PATH                         — set to the pinned root
#   CMAKE_FIND_USE_CMAKE_SYSTEM_PATH  OFF     — disables platform-default prefixes
#   CMAKE_FIND_USE_SYSTEM_PATH        OFF     — disables /usr/lib etc.
#   CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF — disables PATH/LD_LIBRARY_PATH
#
# Always call this near the top of each library processor function (before any
# early return) so that <package_name>_ROOT is always claimed.  This prevents
# the unknown-option check from firing when the user passes
# -D<package_name>_ROOT=... on the command line while the library option is OFF.
#
# All variables set by this function are local to the calling (processor)
# function's scope and automatically disappear when that function returns,
# so they never affect other library searches or the parent CMakeLists.txt scope.
function( NEST_RESTRICT_FIND_IF_PINNED package_name )
  # Promote UNINITIALIZED → PATH so the unknown-option check never fires.
  # Without FORCE, any user-supplied value is preserved; only the type changes.
  set( ${package_name}_ROOT "" CACHE PATH
    "Restrict the ${package_name} search to this installation root (empty = search system)." )

  # Determine the effective root (CMake variable takes precedence over env).
  set( _root "${${package_name}_ROOT}" )
  if ( "${_root}" STREQUAL "" AND DEFINED ENV{${package_name}_ROOT} )
    set( _root "$ENV{${package_name}_ROOT}" )
  endif ()

  if ( NOT "${_root}" STREQUAL "" OR CMAKE_PREFIX_PATH )
    # When the user pinned a specific location, restrict the search to that path
    # only so CMake cannot silently fall back to a system-installed version.
    set( CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF PARENT_SCOPE )
    set( CMAKE_FIND_USE_SYSTEM_PATH OFF PARENT_SCOPE )
    set( CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF PARENT_SCOPE )
    if ( NOT "${_root}" STREQUAL "" )
      # CMAKE_PREFIX_PATH covers config-mode packages and is checked by many
      # module-mode Find<Name>.cmake files; <package>_ROOT (CMP0074) covers the rest.
      set( CMAKE_PREFIX_PATH "${_root}" PARENT_SCOPE )
    endif ()
  endif ()
endfunction()


# Resolve the tristate (OFF | ON | <flags>) value of a compiler-flag option.
# Sets result_var in the caller's scope to:
#   ""                when option_value is OFF-ish,
#   "${default}"      when option_value is ON-ish,
#   "${option_value}" otherwise (use the value directly).
function( NEST_RESOLVE_FLAG_OPTION option_value default_value result_var )
  string( TOUPPER "${option_value}" _upper )
  if ( _upper MATCHES "${_NEST_FALSE_REGEX}" )
    set( ${result_var} "" PARENT_SCOPE )
  elseif ( _upper MATCHES "${_NEST_TRUE_REGEX}" )
    set( ${result_var} "${default_value}" PARENT_SCOPE )
  else ()
    set( ${result_var} "${option_value}" PARENT_SCOPE )
  endif ()
endfunction()

################################################################################
# Compiler flag options (category c)
################################################################################

function( NEST_PROCESS_WITH_OPTIMIZE )
  nest_resolve_flag_option( "${with-optimize}" "-O2" _flags )
  if ( _flags )
    string( JOIN " " _flags_str ${_flags} )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_flags_str}" PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_DEBUG )
  nest_resolve_flag_option( "${with-debug}" "-g" _flags )
  if ( _flags )
    string( JOIN " " _flags_str ${_flags} )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_flags_str}" PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_WARNING )
  nest_resolve_flag_option( "${with-warning}" "-Wall" _flags )
  if ( _flags )
    string( JOIN " " _flags_str ${_flags} )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_flags_str}" PARENT_SCOPE )
  endif ()
endfunction()

# Add -fp-model strict when building with the Intel compiler. Without this flag
# Intel's aggressive floating-point optimisations can alter simulation results.
function( NEST_PROCESS_WITH_INTEL_COMPILER_STRICT_MATH )
  if ( with-intel-compiler-strict-math
       AND CMAKE_CXX_COMPILER_ID STREQUAL "Intel" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fp-model strict" PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_LIBRARIES )
  if ( with-libraries )
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
    foreach ( def ${with-defines} )
      if ( "${def}" MATCHES "^-D.*" )
        add_definitions( "${def}" )
      else ()
        message( FATAL_ERROR "Define '${def}' does not match '-D.*' !" )
      endif ()
    endforeach ()
  endif ()
endfunction()

################################################################################
# Generic build options
################################################################################

function( NEST_GET_COLOR_FLAGS )
  set( NEST_CXX_COLOR_FLAGS "" PARENT_SCOPE )
  if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    set( NEST_CXX_COLOR_FLAGS "-fdiagnostics-color=auto" PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_STATIC_LINKING )
  if ( with-static-linking )
    set( BUILD_SHARED_LIBS OFF PARENT_SCOPE )
    set( CMAKE_SKIP_RPATH TRUE PARENT_SCOPE )

    if ( UNIX OR APPLE )
      # Prefer static library variants when searching for dependencies.
      set( CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib;.dylib;.tbd;.so" PARENT_SCOPE )
    endif ()

    if ( Fugaku )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Bstatic" PARENT_SCOPE )
    else ()
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static" PARENT_SCOPE )
    endif ()

  else ()
    set( BUILD_SHARED_LIBS ON PARENT_SCOPE )
    set( CMAKE_SKIP_RPATH FALSE PARENT_SCOPE )
    set( CMAKE_SKIP_BUILD_RPATH FALSE PARENT_SCOPE )
    set( CMAKE_MACOSX_RPATH ON PARENT_SCOPE )
    set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE PARENT_SCOPE )

    # Note: "$ORIGIN" (on Linux) and "@loader_path" (on macOS) are not CMake
    # variables, but special keywords for the Linux resp. macOS dynamic loader.
    # They refer to the path in which the object is located, e.g.
    # ${CMAKE_INSTALL_PREFIX}/bin for helper scripts and something like
    # ${CMAKE_INSTALL_PREFIX}/lib/python3.x/site-packages/nest for nestkernel_api.so.
    # The RPATH is relative to this origin, so the binary bin/nest can find the
    # files in the relative location ../lib/nest, and similarly for PyNEST and
    # the other libraries. For simplicity, we set all possibilities on all objects.
    if ( APPLE )
      set( CMAKE_INSTALL_RPATH
        # for binaries
        "@loader_path/../${CMAKE_INSTALL_LIBDIR}/nest"
        # for libraries (except pynestkernel)
        "@loader_path/../../${CMAKE_INSTALL_LIBDIR}/nest"
        # for pynestkernel: origin at <prefix>/lib/python3.x/site-packages/nest
        "@loader_path/../../../nest"
        PARENT_SCOPE )
    else ()
      set( CMAKE_INSTALL_RPATH
        # for binaries
        "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/nest"
        # for libraries (except pynestkernel)
        "\$ORIGIN/../../${CMAKE_INSTALL_LIBDIR}/nest"
        # for pynestkernel: origin at <prefix>/lib(64)/python3.x/site-packages/nest
        # while libs are at the root of that at <prefix>/lib(64)/nest
        "\$ORIGIN/../../../nest"
        PARENT_SCOPE )
    endif ()

    set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE PARENT_SCOPE )

    if ( UNIX OR APPLE )
      set( CMAKE_FIND_LIBRARY_SUFFIXES ".so;.dylib;.tbd;.a;.lib" PARENT_SCOPE )
    endif ()
  endif ()

  # Export a clean variable name for configure_file substitution; CMake's
  # configure_file @ONLY does not support variable names containing hyphens.
  set( NEST_WITH_STATIC_LINKING "${with-static-linking}" PARENT_SCOPE )
endfunction()

################################################################################
# Library options (category b)
################################################################################

function( NEST_PROCESS_WITH_LIBLTDL )
  set( HAVE_LIBLTDL OFF PARENT_SCOPE )
  nest_restrict_find_if_pinned( LTDL )  # always: claims LTDL_ROOT
  # ltdl enables dynamic loading of user modules; incompatible with full static linking.
  if ( with-static-linking )
    message( STATUS "LTDL disabled: incompatible with -Dwith-static-linking=ON." )
    return()
  endif ()
  if ( NOT with-ltdl )
    return()
  endif ()
  find_package( LTDL REQUIRED QUIET )
  message( STATUS "Found LTDL: ${LTDL_LIBRARIES} (found version ${LTDL_VERSION})" )
  set( HAVE_LIBLTDL ON PARENT_SCOPE )
  set( LTDL_FOUND ON PARENT_SCOPE )
  set( LTDL_LIBRARIES "${LTDL_LIBRARIES}" PARENT_SCOPE )
  set( LTDL_INCLUDE_DIRS "${LTDL_INCLUDE_DIRS}" PARENT_SCOPE )
  set( LTDL_VERSION "${LTDL_VERSION}" PARENT_SCOPE )
  include_directories( ${LTDL_INCLUDE_DIRS} )
  # is linked in nestkernel/CMakeLists.txt
endfunction()

function( NEST_PROCESS_WITH_GSL )
  set( HAVE_GSL OFF PARENT_SCOPE )
  # GSL_ROOT is the CMP0074 convention variable; CMake searches it in config mode
  # and injects it into find_path/find_library hints inside FindGSL.cmake.
  # FindGSL.cmake additionally checks GSL_ROOT_DIR explicitly; we set that too.
  nest_restrict_find_if_pinned( GSL )
  set( GSL_ROOT_DIR "${GSL_ROOT}" )
  if ( with-gsl )
    find_package( GSL 1.11 REQUIRED QUIET )
    message( STATUS "Found GSL: ${GSL_LIBRARIES} (found version ${GSL_VERSION})" )
    set( HAVE_GSL ON PARENT_SCOPE )
    # export variables needed for nest-config generation
    set( GSL_VERSION "${GSL_VERSION}" PARENT_SCOPE )
    set( GSL_LIBRARIES "${GSL_LIBRARIES}" PARENT_SCOPE )
    set( GSL_INCLUDE_DIRS "${GSL_INCLUDE_DIRS}" PARENT_SCOPE )
    # consumers use GSL::gsl imported target; no global include_directories() needed
  endif ()
  # Provide a dummy GSL::gsl if GSL is disabled so unconditional
  # target_link_libraries() calls do not fail.
  if ( NOT TARGET GSL::gsl )
    add_library( GSL::gsl INTERFACE IMPORTED )
  endif ()
endfunction()

# Resolve the NESTKERNEL_API_CXX variable and set NEST_PREBUILT_PYNEST_CXX
# in the caller's scope:
#   ""             — NESTKERNEL_API_CXX not set: run Cython as normal.
#   "/abs/path/…"  — use that file, skip Cython.  If the file is absent at
#                    configure time a warning is issued at summary time (not fatal,
#                    because the file may be generated by a preceding build step).
# Must be called before NEST_FIND_PYTHON() so the Cython find is skipped when not needed.
function( NEST_PROCESS_WITH_PREBUILT_PYNEST_CXX )
  # Claim NESTKERNEL_API_CXX as a typed variable so a user-supplied
  # -DNESTKERNEL_API_CXX=... does not linger as UNINITIALIZED in the cache.
  # Without FORCE this only promotes the type; any user-supplied value is preserved.
  set( NESTKERNEL_API_CXX "" CACHE FILEPATH
    "Path to a pre-generated nestkernel_api.cxx. When set, Cython is not run." )

  if ( NESTKERNEL_API_CXX )
    set( NEST_PREBUILT_PYNEST_CXX "${NESTKERNEL_API_CXX}" PARENT_SCOPE )
  else ()
    set( NEST_PREBUILT_PYNEST_CXX "" PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_FIND_PYTHON )
  # Python is always required; the with-python option has been removed.
  # IMPORTANT: This function must be called before include(GNUInstallDirs)
  # because it may set CMAKE_INSTALL_PREFIX to the active Python environment root.

  # Declare Python_EXECUTABLE as a typed cache variable so that a user who
  # passes -DPython_EXECUTABLE=... on the command line does not trigger the
  # unknown-option check. Without FORCE this only promotes the TYPE from
  # UNINITIALIZED to FILEPATH while leaving any user-supplied value intact.
  set( Python_EXECUTABLE "" CACHE FILEPATH "Path to Python interpreter." )

  # Development.Module (CMake 3.18+) finds the Python module ABI without
  # requiring the embed library, which PyNEST does not need.
  find_package( Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module )

  if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    execute_process(
      COMMAND "${Python_EXECUTABLE}" "-c"
        "import sys, os; print(int(bool(os.environ.get('VIRTUAL_ENV', False)) or bool(os.environ.get('CONDA_DEFAULT_ENV', False)) or (sys.prefix != sys.base_prefix)))"
      OUTPUT_VARIABLE Python_InVirtualEnv
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    if ( NOT Python_InVirtualEnv AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
      printError(
        "No virtual Python environment found and no installation prefix specified. "
        "Please either build and install NEST in a virtual Python environment or "
        "specify CMake option -DCMAKE_INSTALL_PREFIX=<nest_install_dir>." )
    endif ()

    get_filename_component( Python_EnvRoot "${Python_SITELIB}/../../.." ABSOLUTE )
    set( CMAKE_INSTALL_PREFIX "${Python_EnvRoot}"
      CACHE PATH "Default install prefix for the active Python interpreter" FORCE )
  endif ()

  set( Python_FOUND "${Python_FOUND}" PARENT_SCOPE )
  set( Python_EXECUTABLE "${Python_EXECUTABLE}" PARENT_SCOPE )
  set( PYTHON "${Python_EXECUTABLE}" PARENT_SCOPE )
  set( Python_VERSION "${Python_VERSION}" PARENT_SCOPE )
  set( Python_VERSION_MAJOR "${Python_VERSION_MAJOR}" PARENT_SCOPE )
  set( Python_VERSION_MINOR "${Python_VERSION_MINOR}" PARENT_SCOPE )
  set( Python_INCLUDE_DIRS "${Python_INCLUDE_DIRS}" PARENT_SCOPE )
  set( Python_LIBRARIES "${Python_LIBRARIES}" PARENT_SCOPE )

  # Cython is required only when Cython is to be run (i.e., not using a pre-generated file).
  if ( NEST_PREBUILT_PYNEST_CXX STREQUAL "" )
    find_package( Cython 3.0.0 REQUIRED )
    set( CYTHON_EXECUTABLE "${CYTHON_EXECUTABLE}" PARENT_SCOPE )
    set( CYTHON_VERSION "${CYTHON_VERSION}" PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_SETUP_PYTHON )
  set( PYEXECDIR
    "${CMAKE_INSTALL_LIBDIR}/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages"
    PARENT_SCOPE )
endfunction()

function( NEST_PROCESS_WITH_OPENMP )
  nest_restrict_find_if_pinned( OpenMP )

  if ( NOT with-openmp )
    # Provide a dummy OpenMP::OpenMP_CXX if OpenMP is disabled so unconditional
    # target_link_libraries() calls do not fail.
    if ( NOT TARGET OpenMP::OpenMP_CXX )
      add_library( OpenMP::OpenMP_CXX INTERFACE IMPORTED )
    endif ()
    return()
  endif ()

  # Apple Clang does not bundle libomp. When no explicit OpenMP_ROOT is set,
  # try Homebrew's libomp as an additional search hint.
  if ( APPLE AND "${OpenMP_ROOT}" STREQUAL "" AND NOT DEFINED ENV{OpenMP_ROOT} )
    execute_process(
      COMMAND brew --prefix libomp
      OUTPUT_VARIABLE _brew_libomp_prefix
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      RESULT_VARIABLE _brew_result
    )
    if ( _brew_result EQUAL 0 AND EXISTS "${_brew_libomp_prefix}" )
      set( OpenMP_ROOT "${_brew_libomp_prefix}" )
      # Also update CMAKE_PREFIX_PATH for module-mode Find*.cmake compatibility.
      set( CMAKE_PREFIX_PATH "${_brew_libomp_prefix}" )
    endif ()
  endif ()

  find_package( OpenMP REQUIRED QUIET )
  message( STATUS "Found OpenMP: ${OpenMP_CXX_FLAGS} (found version ${OpenMP_VERSION})" )
  set( OpenMP_FOUND "${OpenMP_FOUND}" PARENT_SCOPE )
  set( OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS}" PARENT_SCOPE )
  set( OpenMP_CXX_LIBRARIES "${OpenMP_CXX_LIBRARIES}" PARENT_SCOPE )
  set( OpenMP_CXX_INCLUDE_DIRS "${OpenMP_CXX_INCLUDE_DIRS}" PARENT_SCOPE )
  # consumers use OpenMP::OpenMP_CXX imported target

  # Provide a dummy OpenMP::OpenMP_CXX if find_package somehow did not create
  # the target (e.g., unsupported compiler) so unconditional
  # target_link_libraries() calls do not fail.
  if ( NOT TARGET OpenMP::OpenMP_CXX )
    add_library( OpenMP::OpenMP_CXX INTERFACE IMPORTED )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_MPI )
  set( HAVE_MPI OFF PARENT_SCOPE )
  nest_restrict_find_if_pinned( MPI )

  if ( with-mpi )
    find_package( MPI REQUIRED QUIET COMPONENTS CXX )
    if ( MPI_CXX_FOUND )
      message( STATUS "Found MPI: ${MPI_CXX_COMPILER} (supports MPI standard ${MPI_CXX_VERSION})" )
      set( HAVE_MPI ON PARENT_SCOPE )
      # export variables needed for nest-config generation and ConfigureSummary
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
      # consumers use MPI::MPI_CXX imported target
    endif ()
  endif ()   # with-mpi

  # Provide a dummy MPI::MPI_CXX if MPI is disabled so unconditional
  # target_link_libraries() calls do not fail.
  if ( NOT TARGET MPI::MPI_CXX )
    add_library( MPI::MPI_CXX INTERFACE IMPORTED )
  endif ()
endfunction()

################################################################################
# Timer options (boolean, category a)
################################################################################

function( NEST_PROCESS_WITH_DETAILED_TIMERS )
  nest_validate_bool_option( with-detailed-timers "${with-detailed-timers}" TIMER_DETAILED )
  set( TIMER_DETAILED ${TIMER_DETAILED} PARENT_SCOPE )
endfunction()

function( NEST_PROCESS_WITH_CYCLE_TIMERS )
  nest_validate_bool_option( with-detailed-timers "${with-detailed-timers}" _detailed_timers )
  nest_validate_bool_option( with-cycle-timers "${with-cycle-timers}" _cycle_timers )

  if ( _cycle_timers AND NOT _detailed_timers )
    message( FATAL_ERROR "To enable cycle timers, you must also enable detailed timers." )
  endif ()

  set( CYCLE_TIMERS ${_cycle_timers} PARENT_SCOPE )
endfunction()

function( NEST_PROCESS_WITH_THREADED_TIMERS )
  nest_validate_bool_option( with-threaded-timers "${with-threaded-timers}" THREADED_TIMERS )
  set( THREADED_TIMERS ${THREADED_TIMERS} PARENT_SCOPE )
endfunction()

function( NEST_PROCESS_WITH_MPI_SYNC_TIMER )
  nest_validate_bool_option( with-mpi-sync-timer "${with-mpi-sync-timer}" MPI_SYNC_TIMER )
  set( MPI_SYNC_TIMER ${MPI_SYNC_TIMER} PARENT_SCOPE )
endfunction()

################################################################################
# Optional external libraries (category b, continued)
################################################################################

function( NEST_PROCESS_WITH_LIBNEUROSIM )
  set( HAVE_LIBNEUROSIM OFF PARENT_SCOPE )
  nest_restrict_find_if_pinned( LibNeurosim )
  if ( NOT with-libneurosim )
    return()
  endif ()
  find_package( LibNeurosim REQUIRED QUIET )
  message( STATUS "Found LibNeurosim: ${LIBNEUROSIM_LIBRARIES} (found version ${LIBNEUROSIM_VERSION})" )
  set( HAVE_LIBNEUROSIM ON PARENT_SCOPE )
  set( LIBNEUROSIM_FOUND "${LIBNEUROSIM_FOUND}" PARENT_SCOPE )
  set( LIBNEUROSIM_LIBRARIES "${LIBNEUROSIM_LIBRARIES}" PARENT_SCOPE )
  set( LIBNEUROSIM_INCLUDE_DIRS "${LIBNEUROSIM_INCLUDE_DIRS}" PARENT_SCOPE )
  set( LIBNEUROSIM_VERSION "${LIBNEUROSIM_VERSION}" PARENT_SCOPE )
  include_directories( ${LIBNEUROSIM_INCLUDE_DIRS} )
  # is linked in nestkernel/CMakeLists.txt
endfunction()

function( NEST_PROCESS_WITH_MUSIC )
  set( HAVE_MUSIC OFF PARENT_SCOPE )
  nest_restrict_find_if_pinned( Music )
  if ( NOT with-music )
    return()
  endif ()
  if ( NOT HAVE_MPI )
    message( FATAL_ERROR "-Dwith-music requires -Dwith-mpi=ON." )
  endif ()
  find_package( Music REQUIRED QUIET )
  message( STATUS "Found MUSIC: ${MUSIC_LIBRARIES} (found version ${MUSIC_VERSION})" )
  set( HAVE_MUSIC ON PARENT_SCOPE )
  set( MUSIC_FOUND "${MUSIC_FOUND}" PARENT_SCOPE )
  set( MUSIC_LIBRARIES "${MUSIC_LIBRARIES}" PARENT_SCOPE )
  set( MUSIC_INCLUDE_DIRS "${MUSIC_INCLUDE_DIRS}" PARENT_SCOPE )
  set( MUSIC_EXECUTABLE "${MUSIC_EXECUTABLE}" PARENT_SCOPE )
  set( MUSIC_VERSION "${MUSIC_VERSION}" PARENT_SCOPE )
  include_directories( ${MUSIC_INCLUDE_DIRS} )
  # is linked in nestkernel/CMakeLists.txt
endfunction()

function( NEST_PROCESS_WITH_SIONLIB )
  set( HAVE_SIONLIB OFF CACHE INTERNAL "sionlib" )
  nest_restrict_find_if_pinned( SIONlib )
  if ( NOT with-sionlib )
    return()
  endif ()
  if ( NOT HAVE_MPI )
    message( FATAL_ERROR "-Dwith-sionlib requires -Dwith-mpi=ON." )
  endif ()
  find_package( SIONlib REQUIRED QUIET )
  message( STATUS "Found SIONlib: ${SIONLIB_LIBRARIES}" )
  set( HAVE_SIONLIB ON CACHE INTERNAL "sionlib" )
  include_directories( ${SIONLIB_INCLUDE} )
  # is linked in nestkernel/CMakeLists.txt
endfunction()

function( NEST_PROCESS_WITH_BOOST )
  set( HAVE_BOOST OFF PARENT_SCOPE )
  nest_restrict_find_if_pinned( Boost )
  if ( with-boost )
    set( Boost_USE_DEBUG_LIBS OFF )
    set( Boost_USE_RELEASE_LIBS ON )
    find_package( Boost 1.70 REQUIRED CONFIG )
    message( STATUS "Found Boost: ${Boost_INCLUDE_DIRS} (found version ${Boost_VERSION_STRING})" )
    set( HAVE_BOOST ON PARENT_SCOPE )
    set( BOOST_FOUND "${Boost_FOUND}" PARENT_SCOPE )
    set( BOOST_VERSION
      "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}" PARENT_SCOPE )
  endif ()
  # Provide a dummy Boost::headers if Boost is disabled so unconditional
  # target_link_libraries() calls do not fail.
  if ( NOT TARGET Boost::headers )
    add_library( Boost::headers INTERFACE IMPORTED )
  endif ()
endfunction()

function( NEST_PROCESS_WITH_HDF5 )
  set( HAVE_HDF5 OFF PARENT_SCOPE )
  nest_restrict_find_if_pinned( HDF5 )
  if ( NOT with-hdf5 )
    return()
  endif ()
  find_package( HDF5 REQUIRED QUIET COMPONENTS C CXX )
  message( STATUS "Found HDF5: ${HDF5_LIBRARIES} (found version ${HDF5_VERSION})" )
  set( HAVE_HDF5 ON PARENT_SCOPE )
  set( HDF5_FOUND "${HDF5_FOUND}" PARENT_SCOPE )
  set( HDF5_LIBRARIES "${HDF5_LIBRARIES}" PARENT_SCOPE )
  set( HDF5_INCLUDE_DIR "${HDF5_INCLUDE_DIRS}" PARENT_SCOPE )
  set( HDF5_VERSION "${HDF5_VERSION}" PARENT_SCOPE )
  set( HDF5_HL_LIBRARIES "${HDF5_HL_LIBRARIES}" PARENT_SCOPE )
  set( HDF5_DEFINITIONS "${HDF5_DEFINITIONS}" PARENT_SCOPE )
  include_directories( ${HDF5_INCLUDE_DIRS} )
endfunction()

################################################################################
# NEST-specific properties
################################################################################

function( NEST_PROCESS_WITH_TARGET_BITS_SPLIT )
  if ( "${with-target-bits-split}" STREQUAL "default" )
    set( TARGET_BITS_SPLIT 0 PARENT_SCOPE )
  elseif ( "${with-target-bits-split}" STREQUAL "hpc" )
    set( TARGET_BITS_SPLIT 1 PARENT_SCOPE )
  else ()
    message( FATAL_ERROR
      "-Dwith-target-bits-split=${with-target-bits-split}: value must be 'default' or 'hpc'." )
  endif ()
endfunction()

function( NEST_PROCESS_MODELS )
  # check mutual exclusivity of -Dwith-models and -Dwith-modelset
  if ( ( NOT with-modelset STREQUAL "full" ) AND with-models )
    printError( "Only one of -Dwith-modelset or -Dwith-models can be specified." )
  endif ()

  if ( with-models )
    set( BUILTIN_MODELS ${with-models} )
  else ()
    if ( NOT EXISTS "${PROJECT_SOURCE_DIR}/modelsets/${with-modelset}" )
      printError( "Cannot find modelset configuration 'modelsets/${with-modelset}'" )
    endif ()
    file( STRINGS "${PROJECT_SOURCE_DIR}/modelsets/${with-modelset}" BUILTIN_MODELS )
  endif ()

  # We use python3 here directly, as some of the CI jobs don't seem to have PYTHON
  # or Python_EXECUTABLE set properly.
  execute_process(
    COMMAND "python3" "${PROJECT_SOURCE_DIR}/build_support/generate_modelsmodule.py"
      "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}" "${BUILTIN_MODELS}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE MODELS_SOURCES
    ERROR_VARIABLE MODELS_SOURCES_ERROR
    # Uncomment for debugging: ECHO_OUTPUT_VARIABLE ECHO_ERROR_VARIABLE COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )

  if ( MODELS_SOURCES_ERROR )
    printError( ${MODELS_SOURCES_ERROR} )
  endif ()

  set( BUILTIN_MODELS ${BUILTIN_MODELS} PARENT_SCOPE )
  set( MODELS_SOURCES_GENERATED ${MODELS_SOURCES} PARENT_SCOPE )
endfunction()

################################################################################
# Documentation options (boolean, category a)
################################################################################

function( NEST_PROCESS_USERDOC )
  nest_validate_bool_option( with-userdoc "${with-userdoc}" _userdoc )
  if ( _userdoc )
    message( STATUS "Configuring user documentation" )
    # QUIET: suppress the module's own "Found" message, we print our own below
    find_package( Sphinx REQUIRED QUIET )
    message( STATUS "Found Sphinx: ${SPHINX_EXECUTABLE}" )
    find_package( Pandoc REQUIRED QUIET )
    message( STATUS "Found Pandoc: ${PANDOC_EXECUTABLE}" )
    set( BUILD_SPHINX_DOCS ON PARENT_SCOPE )
    set( BUILD_DOCS ON PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_DEVDOC )
  nest_validate_bool_option( with-devdoc "${with-devdoc}" _devdoc )
  if ( _devdoc )
    message( STATUS "Configuring developer documentation" )
    # QUIET: suppress the module's own "Found" message, we print our own below
    find_package( Doxygen REQUIRED QUIET COMPONENTS dot )
    message( STATUS "Found Doxygen: ${DOXYGEN_EXECUTABLE} (found version ${DOXYGEN_VERSION})" )
    set( BUILD_DOXYGEN_DOCS ON PARENT_SCOPE )
    set( BUILD_DOCS ON PARENT_SCOPE )
  endif ()
endfunction()

function( NEST_PROCESS_FULL_LOGGING )
  nest_validate_bool_option( with-full-logging "${with-full-logging}" ENABLE_FULL_LOGGING )
  if ( ENABLE_FULL_LOGGING )
    message( STATUS "Configuring full logging" )
  endif ()
  set( ENABLE_FULL_LOGGING ${ENABLE_FULL_LOGGING} PARENT_SCOPE )
endfunction()
