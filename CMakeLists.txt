# CMakeLists.txt
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
cmake_minimum_required( VERSION 2.8.12 )
# juqueen 2.8.12.2

# add cmake modules
set( CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules" )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Platform )

project( nest CXX C )
set( NEST_USER_EMAIL "nest_user@nest-initiative.org" )

# VERSION exported to libnestutil/config.h, extras/create_release.sh
set( NEST_VERSION_MAJOR 2 )
set( NEST_VERSION_MINOR 10 )
set( NEST_VERSION_PATCHLEVEL 0 )

set( NEST_VERSION_VERSION "${NEST_VERSION_MAJOR}.${NEST_VERSION_MINOR}.${NEST_VERSION_PATCHLEVEL}" )
set( NEST_VERSION_PRGNAME "${PROJECT_NAME}-${NEST_VERSION_VERSION}" )

# some default directories
# install prefix is set with -DCMAKE_INSTALL_PREFIX:PATH=...
set( INSTALL_BIN_DIR bin )
set( INSTALL_LIB_DIR lib )
set( INSTALL_INC_DIR include )
set( INSTALL_DATA_DIR share/nest )
set( INSTALL_DOC_DIR share/doc/nest )

set( PKGSRCDIR ${PROJECT_SOURCE_DIR} )
set( PKGBUILDDIR ${PROJECT_BINARY_DIR} )
set( PKGDATADIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA_DIR} )
set( PKGDOCDIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR} )
set( TESTSUITE_BASEDIR ${PKGDOCDIR} )

# on OS X
set( CMAKE_MACOSX_RPATH ON )

# set the rpath only when installed
# https://cmake.org/Wiki/CMake_RPATH_handling#Mac_OS_X_and_the_RPATH
set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}" "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}/nest" )

# CPack stuff
set( CPACK_GENERATOR TGZ )
set( CPACK_SOURCE_GENERATOR TGZ )

set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "nest::The Neural Simulation Tool" )
set( CPACK_PACKAGE_VENDOR "NEST Initiative (http://www.nest-initiative.org/)" )
set( CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/extras/logos/nest.png" )

set( CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE" )
set( CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md" )

set( CPACK_PACKAGE_VERSION_MAJOR ${NEST_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${NEST_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${NEST_VERSION_PATCHLEVEL} )
set( CPACK_PACKAGE_VERSION ${NEST_VERSION_VERSION} )
set( CPACK_PACKAGE_EXECUTABLES "nest" )

set( CPACK_SOURCE_IGNORE_FILES
    "\\\\.gitignore"
    "\\\\.git/"
    "\\\\.travis\\\\.yml"
    # ignore autotools stuff
    # TODO cleanup, once autotools stuff is removed
    "/libltdl/"
    "/debian/"
    ".*\\\\.m4"
    ".*\\\\.ac"
    "autom4te\\\\.cache/"
    "bootstrap\\\\.sh"

    # if we have in source builds
    "/build/"
    "/_CPack_Packages/"
    "CMakeFiles/"
    "cmake_install\\\\.cmake"
    "Makefile.*"
    "CMakeCache\\\\.txt"
    "CPackConfig\\\\.cmake"
    "CPackSourceConfig\\\\.cmake"
    )
set( CPACK_SOURCE_PACKAGE_FILE_NAME ${NEST_VERSION_PRGNAME} )

set( CPACK_PACKAGE_INSTALL_DIRECTORY "NEST ${NEST_VERSION_VERSION}" )
include( CPack )

################################################################################
#################        Do not install into /usr/local        #################
################################################################################

if ( ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local" )
  # no prefix given?
  message( FATAL_ERROR "Do not install into '${CMAKE_INSTALL_PREFIX}'? Please set -DCMAKE_INSTALL_PREFIX:PATH=<install prefix>." )
endif ()

################################################################################
#################    Check for includes/ functions /symbols    #################
################################################################################

include( CheckIncludeFiles )
check_include_files( "inttypes.h" HAVE_INTTYPES_H )
check_include_files( "mach-o/dyld.h" HAVE_MACH_O_DYLD_H )
check_include_files( "mach/mach.h" HAVE_MACH_MACH_H )
check_include_files( "memory.h" HAVE_MEMORY_H )
check_include_files( "stdint.h" HAVE_STDINT_H )
check_include_files( "sys/types.h" HAVE_SYS_TYPES_H )

include( CheckIncludeFileCXX )
check_include_file_cxx( "istream" HAVE_ISTREAM )
check_include_file_cxx( "ostream" HAVE_OSTREAM )
check_include_file_cxx( "sstream" HAVE_SSTREAM )

# Check types exist
include( CheckTypeSize )
check_type_size( "long long" LONG_LONG_SIZE ) # also sets HAVE_LONG_LONG
if ( LONG_LONG_SIZE GREATER 0 )
  set( HAVE_LONG_LONG ON )
endif ()

set( CMAKE_EXTRA_INCLUDE_FILES "types.h" )
check_type_size( u_int16_t U_INT16_T_SIZE )
set( CMAKE_EXTRA_INCLUDE_FILES OFF )
if ( U_INT16_T_SIZE GREATER 0 )
  set( HAVE_U_INT16_T ON )
endif ()

set( CMAKE_EXTRA_INCLUDE_FILES "stdint.h" )
check_type_size( uint16_t UINT16_T_SIZE )
set( CMAKE_EXTRA_INCLUDE_FILES OFF )
if ( UINT16_T_SIZE GREATER 0 )
  set( HAVE_UINT16_T ON )
endif ()

# Check symbols / defines exist
include( CheckSymbolExists )
check_symbol_exists( NAN "math.h" HAVE_NAN )
check_symbol_exists( isnan "math.h" HAVE_ISNAN )

include( CheckCXXSymbolExists )
check_cxx_symbol_exists( M_E "cmath" HAVE_M_E )
check_cxx_symbol_exists( M_PI "cmath" HAVE_M_PI )

# Check functions exist
include( CheckFunctionExists )
check_function_exists( expm1 "math.h" HAVE_EXPM1 )

################################################################################
##################         All User Defined options           ##################
################################################################################

# change NEST defaults
set( tics_per_ms "1000.0" CACHE STRING "Specify elementary unit of time. [default 1000.0]" )
set( tics_per_step "100" CACHE STRING "Specify resolution. [default 100]" )
option( with-ps-arrays "Use PS array construction semantics. [default=ON]" ON )

# add user modules
option( external-modules "External modules to be linked in. Divided by ';'. [default=OFF]" OFF )

# connect NEST with external projects
option( with-libneurosim "Request the use of libneurosim. Optionally give the directory, where libneurosim is installed. [default=OFF]" OFF )
option( with-music "Request the use of MUSIC. Optionally give the directory, where MUSIC is installed. [default=OFF]" OFF )

# change parallelization scheme
option( with-mpi "Request compilation with MPI. Optionally give directory with MPI installation. [default=OFF]" OFF )
option( with-openmp "Enable OpenMP multithreading. Optional: set OMP flag. [default=ON]" ON )

# define default libraries
option( with-gsl "Find a gsl library. To set a specific gsl installation, set install path. [default=ON]" ON)
option( with-readline "Find a readline library. To set a specific readline, set install path. [default=ON]" ON )
option( with-python "Build PyNEST. To set a specific Python, set install path. [default=ON]" ON )

# Whether to build a 'mostly' static executable and static libraries.
option( static-libraries "Build static executable and libraries. [default=OFF]" OFF )
# additional compile flags
option( with-optimize "Enable user defined optimizations. [default OFF, when ON, defaults to '-O3']" OFF )
option( with-warning "Enable user defined warnings. [default ON, when ON, defaults to '-Wall']" ON )
option( with-debug "Enable user defined debug flags. [default OFF, when ON, defaults to '-g']" OFF )
set( with-libraries OFF CACHE STRING "Link additional libraries. Give full path. Separate multiple libraries by ';'. [default OFF]" )
set( with-includes OFF CACHE STRING "Add additional include paths. Give full path without '-I'. Separate multiple include paths by ';'. [default OFF]" )
set( with-defines OFF CACHE STRING "Additional defines, e.g. '-DXYZ=1'. Separate multiple defines by ';'. [default OFF]" )

# cross-compiling
# should be set via toolchain files
set( enable-bluegene OFF CACHE STRING "Configure for BlueGene." )
set( k-computer OFF CACHE BOOL "Enable K computer." )

################################################################################
##################           Find utility programs            ##################
################################################################################

# needed for target doc and fulldoc
find_package( Doxygen )
find_program( SED NAMES sed gsed )

################################################################################
##################          Process custom options            ##################
################################################################################

# add custom warnings and optimizations
if ( with-optimize )
  if ( with-optimize STREQUAL "ON" )
    set( with-optimize "-O3" )
  endif ()
  foreach ( flag ${with-optimize} )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" )
  endforeach ()
endif ( with-optimize )

if ( with-debug )
  if ( with-debug STREQUAL "ON" )
    set( with-debug "-g" )
  endif ()
  foreach ( flag ${with-debug} )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" )
  endforeach ()
endif ( with-debug )

if ( with-warning )
  if ( with-warning STREQUAL "ON" )
    set( with-warning "-Wall" )
  endif ()
  foreach ( flag ${with-warning} )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" )
  endforeach ()
endif ( with-warning )

if ( with-libraries )
  if ( with-libraries STREQUAL "ON" )
    message(FATAL_ERROR "-Dwith-libraries requires full library paths.")
  endif ()
  foreach ( lib ${with-libraries} )
    if ( EXISTS "${lib}" )
      link_libraries( "${lib}" )
    else ()
      message(FATAL_ERROR "Library '${lib}' does not exist!")
    endif ()
  endforeach ()
endif ( with-libraries )

if ( with-includes )
  if ( with-includes STREQUAL "ON" )
    message(FATAL_ERROR "-Dwith-includes requires full paths.")
  endif ()
  foreach ( inc ${with-includes} )
    if ( IS_DIRECTORY "${inc}" )
      include_directories( "${inc}" )
    else ()
      message(FATAL_ERROR "Include path '${inc}' does not exist!")
    endif ()
  endforeach ()
endif ( with-includes )

if ( with-defines )
  if ( with-defines STREQUAL "ON" )
    message(FATAL_ERROR "-Dwith-defines requires compiler defines -DXYZ=... .")
  endif ()
  foreach ( def ${with-defines} )
    if ( "${def}" MATCHES "^-D.*")
      add_definitions( "${def}" )
    else ()
      message(FATAL_ERROR "Define '${def}' does not match '-D.*' !")
    endif ()
  endforeach ()
endif ( with-defines )

if ( k-computer )
  # TODO use the -DCMAKE_TOOLCHAIN_FILE= argument
  set( IS_K ON )
  # need alternative tokens command to compile NEST
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --alternative_tokens")
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --alternative_tokens")
endif ( k-computer )

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
endif ( enable-bluegene )

# build static or shared libraries
if ( static-libraries )
  set( BUILD_SHARED_LIBS OFF )
  if ( UNIX OR APPLE )
    # On Linux .a is the static library suffix, on Mac OS X .lib can also
    # be used, so we'll add both to the preference list.
    set( CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib;.dylib;.so" )
  endif ()
else ()
  set( BUILD_SHARED_LIBS ON )
  if ( UNIX OR APPLE )
    # reverse the search order for lib extensions
    set( CMAKE_FIND_LIBRARY_SUFFIXES ".so;.dylib;.a;.lib" )
  endif ()
endif ()

# Static modules
set( SLI_MODULES models precise topology )
if ( with-libneurosim )
  set( SLI_MODULES ${SLI_MODULES} conngen )
endif ( with-libneurosim )

set( SLI_MODULE_INCLUDE_DIRS )
foreach ( mod ${SLI_MODULES} )
  list( APPEND SLI_MODULE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/${mod}" )
endforeach ()

if ( external-modules )
  # headers from external modules will be installed here
  include_directories( "${CMAKE_INSTALL_PREFIX}/${INSTALL_INC_DIR}" )

  # put all external libs into this variable
  set( EXTERNAL_MODULE_LIBRARIES )
  # put all external headers into this variable
  set( EXTERNAL_MODULE_INCLUDES )
  foreach ( mod ${external-modules} )
    # find module header
    find_file( ${mod}_EXT_MOD_INCLUDE
        NAMES ${mod}module.h
        HINTS "${CMAKE_INSTALL_PREFIX}/${INSTALL_INC_DIR}"
        )
    if ( ${mod}_EXT_MOD_INCLUDE STREQUAL "${mod}_EXT_MOD_INCLUDE-NOTFOUND" )
      message( FATAL_ERROR "Cannot find header for external module '${mod}'. Should be '${CMAKE_INSTALL_PREFIX}/${INSTALL_INC_DIR}/${mod}module.h' ." )
    endif ()
    list( APPEND EXTERNAL_MODULE_INCLUDES ${${mod}_EXT_MOD_INCLUDE} )

    # find module library
    find_library( ${mod}_EXT_MOD_LIBRARY
        NAMES ${mod}module
        HINTS ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}/nest
        )
    if ( ${mod}_EXT_MOD_LIBRARY STREQUAL "${mod}_EXT_MOD_LIBRARY-NOTFOUND" )
      message( FATAL_ERROR "Cannot find library for external module '${mod}'." )
    endif ()
    list( APPEND EXTERNAL_MODULE_LIBRARIES "${${mod}_EXT_MOD_LIBRARY}" )
  endforeach ( mod )
endif ( external-modules )


# write static_modules.h
file( WRITE ${PROJECT_BINARY_DIR}/nest/static_modules.h "#ifndef STATIC_MODULES_H\n" )
file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#define STATIC_MODULES_H\n\n" )
file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "// Add all in source modules:\n" )
foreach ( mod ${SLI_MODULES} )
  file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#include \"${mod}module.h\"\n" )
endforeach ( mod )

# when we build statically, we need to add headers and addmodule for external modules
# just as if it were a in source module.
if ( static-libraries AND external-modules )
  file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "\n// Add all external modules:\n" )
  foreach ( mod ${external-modules} )
    file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#include \"${mod}module.h\"\n" )
  endforeach ( mod )
endif ( static-libraries AND external-modules )
file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "\n#include \"interpret.h\"\n\n" )
file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "void add_static_modules(SLIInterpreter& engine)\n{\n" )
file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "  // Add all in source modules:\n" )
foreach ( mod ${SLI_MODULES} )
  # get class name, is always in nest namespace
  file( STRINGS "${PROJECT_SOURCE_DIR}/${mod}/${mod}module.h" module_class_string REGEX "class.*: public SLIModule" )
  string( REGEX REPLACE "class ([a-zA-Z0-9_]+) : public SLIModule" "\\1" module_class ${module_class_string} )
  file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "  engine.addmodule(new nest::${module_class}());\n" )
endforeach ()

# when we build statically, we need to add headers and addmodule for external modules
# just as if it were a in source module.
if ( static-libraries AND external-modules )
  file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "\n  // Add all external modules:\n" )
  foreach ( mod_header ${EXTERNAL_MODULE_INCLUDES} )
    # get namespace:
    file( STRINGS "${mod_header}" module_namespace_string REGEX "^namespace.*" )
    if ( NOT module_namespace_string )
      message( FATAL_ERROR "Could not find namespace in '${mod_header}'." )
    endif ()
    string( REGEX REPLACE "namespace ([a-zA-Z0-9_]+)" "\\1" module_namespace ${module_namespace_string} )

    # get class name
    file( STRINGS "${mod_header}" module_class_string REGEX "^class.*: public SLIModule" )
    if ( NOT module_class_string )
      message( FATAL_ERROR "Could not find class that extends SLIModule in '${mod_header}'." )
    endif ()
    string( REGEX REPLACE "class ([a-zA-Z0-9_]+) : public SLIModule" "\\1" module_class ${module_class_string} )
    file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "  engine.addmodule(new ${module_namespace}::${module_class}());\n" )
  endforeach ()
endif ( static-libraries AND external-modules )

file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "}\n\n#endif\n" )
# write static_modules.h

# Set tics per ms / step
if ( tics_per_ms )
  set( HAVE_TICS_PER_MS ON )
endif ( tics_per_ms )

if ( tics_per_step )
  set( HAVE_TICS_PER_STEP ON )
endif ( tics_per_step )

if ( with-ps-arrays )
  set( PS_ARRAYS ON )
endif ( with-ps-arrays )
################################################################################
##################            Find Libraries here             ##################
################################################################################

# Only find libLTDL if we link dynamically
set( HAVE_LIBLTDL OFF )
if ( NOT static-libraries )
  find_package( LTDL )
  if ( LTDL_FOUND )
    set( HAVE_LIBLTDL ON )

    include_directories( ${LTDL_INCLUDE_DIRS} )
    link_libraries( ${LTDL_LIBRARIES} )
  endif ( LTDL_FOUND )
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
    link_libraries( ${READLINE_LIBRARIES} )
  endif ( READLINE_FOUND )
endif ()

# Find GSL
set( HAVE_GSL OFF )
set( HAVE_GSL_1_2 OFF )
set( HAVE_GSL_1_11 OFF )
if ( with-gsl )
  if ( NOT ${with-gsl} STREQUAL "ON" )
    # if set, use this prefix
    set( GSL_ROOT_DIR ${with-gsl} )
  endif ()

  find_package( GSL )

  if ( GSL_FOUND )
    set( HAVE_GSL ON )

    if ( "${GSL_VERSION}" VERSION_GREATER "1.2" )
      set( HAVE_GSL_1_2 ON )
    endif ()

    if ( "${GSL_VERSION}" VERSION_GREATER "1.11" )
      set( HAVE_GSL_1_11 ON )
    endif ()
    include_directories( ${GSL_INCLUDE_DIRS} )
    link_libraries( ${GSL_LIBRARIES} )
  endif ( GSL_FOUND )
endif ( with-gsl )

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

      find_package( Cython )
      if ( CYTHON_FOUND )
        # confirmed not working: 0.15.1
        # confirmed working: 0.19.2+
        # in between unknown
        if ( CYTHON_VERSION VERSION_LESS "0.19.2" )
          message( FATAL_ERROR "Your Cython version is too old. Please install newer version (0.19.2+)" )
        endif ()
      endif ( CYTHON_FOUND )

      set( PYEXECDIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages )
    endif ()
  endif ()
endif ( with-python )

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
    endif ( OPENMP_FOUND )
  endif ()
endif ( with-openmp )

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
    link_libraries( ${MPI_CXX_LIBRARIES} )
  endif ( MPI_CXX_FOUND )
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
    link_libraries( ${LIBNEUROSIM_LIBRARIES} )
  endif ( LIBNEUROSIM_FOUND )
endif ( with-libneurosim )

# Find music
set( HAVE_MUSIC OFF )
if ( with-music )
  if ( NOT ${with-music} STREQUAL "ON" )
    # a path is set
    set( MUSIC_ROOT_DIR "${with-music}" )
  endif ()

  if ( NOT HAVE_MPI )
    message( FATAL_ERROR "MUSIC requires -Dwith-mpi=ON." )
  endif ( NOT HAVE_MPI )

  find_package( Music )
  include_directories( ${MUSIC_INCLUDE_DIRS} )
  link_libraries( ${MUSIC_LIBRARIES} )
  if ( MUSIC_FOUND )
    set( HAVE_MUSIC ON )
  endif ( MUSIC_FOUND )
endif ( with-music )

################################################################################
##################        Define Subdirectories here          ##################
################################################################################
enable_testing()
set( TEST_LOGFILE ${PROJECT_BINARY_DIR}/reports/installcheck.log )
file( MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/reports/ )

add_subdirectory( cmake )
if ( HAVE_LIBNEUROSIM )
  add_subdirectory( conngen )
endif ( HAVE_LIBNEUROSIM )
add_subdirectory( doc )
add_subdirectory( examples )
add_subdirectory( extras )
add_subdirectory( lib )
add_subdirectory( libnestutil )
add_subdirectory( librandom )
add_subdirectory( models )
add_subdirectory( sli )
add_subdirectory( nest )
add_subdirectory( nestkernel )
add_subdirectory( precise )
add_subdirectory( testsuite )
add_subdirectory( topology )
if ( HAVE_PYTHON )
  add_subdirectory( pynest )
endif ( HAVE_PYTHON )

################################################################################
##################           Summary of flags                 ##################
################################################################################

# used in nest-config

# all compiler flags
if ( NOT CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "None" )
  set( ALL_CFLAGS "${CMAKE_C_FLAGS}" )
  set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS}" )
elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
  set( ALL_CFLAGS "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_DEBUG}" )
  set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}" )
elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "Release" )
  set( ALL_CFLAGS "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_RELEASE}" )
  set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}" )
elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo" )
  set( ALL_CFLAGS "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
  set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
elseif ( ${CMAKE_BUILD_TYPE} STREQUAL "MinSizeRel" )
  set( ALL_CFLAGS "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_MINSIZEREL}" )
  set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_MINSIZEREL}" )
else ()
  message( FATAL_ERROR "Unknown build type: '${CMAKE_BUILD_TYPE}'" )
endif ()
if ( with-defines )
  foreach( def ${with-defines} )
    set( ALL_CFLAGS "${def} ${ALL_CFLAGS}")
    set( ALL_CXXFLAGS "${def} ${ALL_CXXFLAGS}")
  endforeach ()
endif ( with-defines )

# all libraries
set( ALL_LIBS "${OpenMP_CXX_FLAGS};${LTDL_LIBRARIES};${READLINE_LIBRARIES};${GSL_LIBRARIES};${LIBNEUROSIM_LIBRARIES};${MUSIC_LIBRARIES};${MPI_CXX_LIBRARIES}" )
if ( with-libraries )
  set( ALL_LIBS "${ALL_LIBS};${with-libraries}" )
endif ( with-libraries )
string( REPLACE ";" " " ALL_LIBS "${ALL_LIBS}" )

# all includes
set( ALL_INCLUDES_tmp "${LTDL_INCLUDE_DIRS};${READLINE_INCLUDE_DIRS};${GSL_INCLUDE_DIRS};${LIBNEUROSIM_INCLUDE_DIRS};${MUSIC_INCLUDE_DIRS};${MPI_CXX_INCLUDE_PATH}" )
set( ALL_INCLUDES "" )
foreach ( INC ${ALL_INCLUDES_tmp} ${with-includes} )
  if ( INC AND NOT INC STREQUAL "" )
    set( ALL_INCLUDES "${ALL_INCLUDES} -I${INC}" )
  endif ()
endforeach ()

if ( HAVE_LIBNEUROSIM )
  set( ALL_LIBS "-lconngen ${ALL_LIBS}" )
  set( ALL_INCLUDES "-I${PROJECT_SOURCE_DIR}/conngen ${ALL_INCLUDES}" )
endif ( HAVE_LIBNEUROSIM )

################################################################################
##################           File generation here             ##################
################################################################################

configure_file(
    "${PROJECT_SOURCE_DIR}/libnestutil/config.h.in"
    "${PROJECT_BINARY_DIR}/libnestutil/config.h" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/pynest/setup.py.in"
    "${PROJECT_BINARY_DIR}/pynest/setup.py" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/pynest/do_tests.py.in"
    "${PROJECT_BINARY_DIR}/pynest/do_tests.py" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/topology/setup.py.in"
    "${PROJECT_BINARY_DIR}/topology/setup.py" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/testsuite/do_tests.sh.in"
    "${PROJECT_BINARY_DIR}/testsuite/do_tests.sh" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/extras/emacs/sli.el.in"
    "${PROJECT_BINARY_DIR}/extras/emacs/sli.el" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/extras/nest-config.in"
    "${PROJECT_BINARY_DIR}/extras/nest-config" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/extras/create_release.sh.in"
    "${PROJECT_BINARY_DIR}/extras/create_release.sh" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/extras/nest_vars.sh.in"
    "${PROJECT_BINARY_DIR}/extras/nest_vars.sh" @ONLY
)

################################################################################
##################           Custom Global Targets            ##################
################################################################################

if ( HAVE_PYTHON )
  add_custom_target( installcheck
      #COMMAND ${CMAKE_MAKE_PROGRAM} install
      COMMAND export PATH="${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}:$ENV{PATH}" &&
      export PYTHON="${PYTHON}" &&
      export PYTHONPATH="${PYEXECDIR}:$ENV{PYTHONPATH}" &&
      export exec_prefix="${CMAKE_INSTALL_PREFIX}" &&
      ${PKGDATADIR}/extras/do_tests.sh --test-pynest --source-dir="${PROJECT_SOURCE_DIR}"
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
      COMMENT "Execute NEST's testsuite...."
      )
else ( HAVE_PYTHON )
  add_custom_target( installcheck
      #COMMAND ${CMAKE_MAKE_PROGRAM} install
      COMMAND export PATH="${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}:$ENV{PATH}" &&
      ${PKGDATADIR}/extras/do_tests.sh --source-dir="${PROJECT_SOURCE_DIR}"
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
      COMMENT "Execute NEST's testsuite...."
      )
endif ( HAVE_PYTHON )

add_custom_target( check
    COMMAND export CTEST_OUTPUT_ON_FAILURE=1 &&
    ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE}
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    )

# add make dist target
add_custom_target( dist
    # create current rcsinfo.sli
    # target dir is PROJECT_SOURCE_DIR as this will be the `install dir` from cmake package_source
    COMMAND ${PROJECT_SOURCE_DIR}/extras/create_rcsinfo.sh "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}"
    COMMAND ${CMAKE_MAKE_PROGRAM} package_source
    # not sure about this... seems, that it will be removed before dist...
    # DEPENDS doc
    COMMENT "Creating a source distribution from NEST..."
    )

install( FILES LICENSE README.md NEWS
    DESTINATION ${INSTALL_DOC_DIR}
    )

################################################################################
##################           Configuration Summary            ##################
################################################################################

message( "" )
message( "--------------------------------------------------------------------------------" )
message( "NEST Configuration Summary" )
message( "--------------------------------------------------------------------------------" )
message( "" )
message( "Build type          : ${CMAKE_BUILD_TYPE}" )
message( "Target System       : ${CMAKE_SYSTEM_NAME}" )
message( "C compiler          : ${CMAKE_C_COMPILER}" )
message( "C compiler flags    : ${ALL_CFLAGS}" )
message( "C++ compiler        : ${CMAKE_CXX_COMPILER}" )
message( "C++ compiler flags  : ${ALL_CXXFLAGS}" )
message( "Build dynamic       : ${BUILD_SHARED_LIBS}" )
message( "" )
message( "Built-in modules    : ${SLI_MODULES}" )
if ( external-modules )
  message( "User modules        : ${external-modules}" )
  foreach ( mod ${external-modules} )
    message( "  ${mod}:" )
    message( "    Header          : ${${mod}_EXT_MOD_INCLUDE}" )
    message( "    Library         : ${${mod}_EXT_MOD_LIBRARY}" )
    message( "" )
  endforeach ()
else ( external-modules )
  message( "User modules        : None" )
endif ( external-modules )

if ( HAVE_PYTHON )
  message( "Python bindings     : Yes (Python ${PYTHON_VERSION}: ${PYTHON})" )
  message( "Cython bindings     : Yes (Cython ${CYTHON_VERSION}: ${CYTHON_EXECUTABLE})" )
else ( HAVE_PYTHON )
  message( "Python bindings     : No" )
endif ( HAVE_PYTHON )

if ( OPENMP_FOUND )
  message( "Use threading       : Yes (OpenMP: ${OpenMP_CXX_FLAGS})" )
else ( OPENMP_FOUND )
  message( "Use threading       : No" )
endif ( OPENMP_FOUND )

if ( HAVE_GSL )
  message( "Use GSL             : Yes (GSL ${GSL_VERSION})" )
  message( "    Includes        : ${GSL_INCLUDE_DIRS}" )
  message( "    Libraries       : ${GSL_LIBRARIES}" )
  message( "" )
else ( HAVE_GSL )
  message( "Use GSL             : No" )
endif ( HAVE_GSL )

if ( HAVE_READLINE )
  message( "Use Readline        : Yes (GNU Readline ${READLINE_VERSION})" )
  message( "    Includes        : ${READLINE_INCLUDE_DIRS}" )
  message( "    Libraries       : ${READLINE_LIBRARIES}" )
  message( "" )
else ( HAVE_READLINE )
  message( "Use Readline        : No" )
endif ( HAVE_READLINE )

if ( HAVE_LIBLTDL )
  message( "Use libltdl         : Yes (LTDL ${LTDL_VERSION})" )
  message( "    Includes        : ${LTDL_INCLUDE_DIRS}" )
  message( "    Libraries       : ${LTDL_LIBRARIES}" )
  message( "" )
else ( HAVE_LIBLTDL )
  message( "Use libltdl         : No" )
endif ( HAVE_LIBLTDL )

if ( DOXYGEN_FOUND )
  message( "Use doxygen         : Yes (${DOXYGEN_EXECUTABLE})" )
  message( "                    : target `doc` available" )
  if ( DOXYGEN_DOT_FOUND )
    message( "    `dot` available : Yes (${DOXYGEN_DOT_EXECUTABLE})" )
    message( "                    : target `fulldoc` available" )
  endif ( DOXYGEN_DOT_FOUND )
else ( DOXYGEN_FOUND )
  message( "Use doxygen         : No" )
endif ( DOXYGEN_FOUND )

if ( HAVE_MPI )
  message( "Use MPI             : Yes (MPI: ${MPI_CXX_COMPILER})" )
  message( "    FLAGS           : ${MPI_CXX_COMPILE_FLAGS}" )
  message( "    Includes        : ${MPI_CXX_INCLUDE_PATH}" )
  message( "    Link Flags      : ${MPI_CXX_LINK_FLAGS}" )
  message( "    Libraries       : ${MPI_CXX_LIBRARIES}" )
  message( "" )
else ( HAVE_MPI )
  message( "Use MPI             : No" )
endif ( HAVE_MPI )

if ( HAVE_MUSIC )
  message( "Use MUSIC           : Yes (MUSIC ${MUSIC_VERSION})" )
  message( "    Includes        : ${MUSIC_INCLUDE_DIRS}" )
  message( "    Libraries       : ${MUSIC_LIBRARIES}" )
  message( "" )
else ( HAVE_MUSIC )
  message( "Use MUSIC           : No" )
endif ( HAVE_MUSIC )

if ( HAVE_LIBNEUROSIM )
  message( "Use libneurosim     : Yes (LibNeurosim ${LIBNEUROSIM_VERSION})" )
  message( "    Includes        : ${LIBNEUROSIM_INCLUDE_DIRS}" )
  message( "    Libraries       : ${LIBNEUROSIM_LIBRARIES}" )
  message( "" )
else ( HAVE_LIBNEUROSIM )
  message( "Use libneurosim     : No" )
endif ( HAVE_LIBNEUROSIM )

if ( with-libraries )
  message( "Additional libraries:" )
  foreach ( lib ${with-libraries} )
    message( "                     ${lib}" )
  endforeach ()
endif ( with-libraries )

if ( with-includes )
  message( "Additional includes:" )
  foreach ( inc ${with-includes} )
    message( "                     -I${inc}" )
  endforeach ()
endif ( with-includes )

message( "" )
message( "--------------------------------------------------------------------------------" )
message( "" )
message( "The NEST executable will be installed to:" )
message( "  ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}/" )
message( "" )
message( "Documentation and examples will be installed to:" )
message( "  ${PKGDOCDIR}/" )
message( "" )
if ( HAVE_PYTHON )
  message( "PyNEST will be installed to:" )
  message( "    ${PYEXECDIR}" )
  message( "" )
endif ( HAVE_PYTHON )
message( "To set necessary environment variables, add the following line" )
message( "to your ~/.bashrc :" )
message( "  source ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}/nest_vars.sh" )
message( "" )
message( "--------------------------------------------------------------------------------" )
message( "" )

if ( HAVE_GSL )
  if ( NOT HAVE_GSL_1_2 )
    message( "" )
    message( "Your GNU Scientific Library is earlier than GSL v 1.2." )
    message( "GSL Random Number Generators will not be available." )
    message( "See http://www.gnu.org/software/gsl for newer versions." )
    message( "" )
    message( "--------------------------------------------------------------------------------" )
    message( "" )
  endif ( NOT HAVE_GSL_1_2 )
  if ( NOT HAVE_GSL_1_11 )
    message( "" )
    message( "Your GNU Scientific Library is earlier than GSL v 1.11." )
    message( "The aeif_cond_alpha neuron will not be available." )
    message( "See http://www.gnu.org/software/gsl for newer versions." )
    message( "" )
    message( "--------------------------------------------------------------------------------" )
    message( "" )
  endif ( NOT HAVE_GSL_1_11 )
else ()
  message( "" )
  message( "ATTENTION!" )
  message( "You are about to compile NEST without the GNU Scientific" )
  message( "Library. This means that conductance-based neuron models" )
  message( "and some random number generators will not be available." )
  message( "" )
  message( "--------------------------------------------------------------------------------" )
  message( "" )
endif ( HAVE_GSL )

message( "You can now build and install NEST with" )
message( "  make" )
message( "  make install" )
message( "  make installcheck" )
message( "" )

if ( HAVE_MPI )
  message( "You have configured NEST with support for distributed computing." )
  message( "After running make install, please tell NEST how to start a" )
  message( "distributed job on your system before running make installcheck." )
  message( "You can do this by modifying the template for command /mpirun in" )
  message( "~/.nestrc, which is created by the first start of NEST." )
  message( "" )
endif ( HAVE_MPI )

message( "If you experience problems with the installation or the use of NEST," )
message( "please see http://www.nest-simulator.org/frequently_asked_questions" )
message( "or go to http://www.nest-simulator.org/community to find out how to" )
message( "join the user mailing list." )
message( "" )
