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
check_include_files( "assert.h" HAVE_ASSERT_H )
check_include_files( "crt_externs.h" HAVE_CRT_EXTERNS_H )
check_include_files( "dirent.h" HAVE_DIRENT_H )
check_include_files( "errno.h" HAVE_ERRNO_H )
check_include_files( "fcntl.h" HAVE_FCNTL_H )
check_include_files( "inttypes.h" HAVE_INTTYPES_H )
check_include_files( "kernel/memory.h" HAVE_KERNEL_MEMORY_H )
check_include_files( "mach-o/dyld.h" HAVE_MACH_O_DYLD_H )
check_include_files( "mach/mach.h" HAVE_MACH_MACH_H )
check_include_files( "math.h" HAVE_MATH_H )
check_include_files( "memory.h" HAVE_MEMORY_H )
check_include_files( "regex.h" HAVE_REGEX_H )
check_include_files( "signal.h" HAVE_MEMORY_H )
check_include_files( "spi/bgp_SPI.h" HAVE_SPI_BGP_SPI_H )
check_include_files( "stddef.h" HAVE_STDDEF_H )
check_include_files( "stdint.h" HAVE_STDINT_H )
check_include_files( "stdio.h" HAVE_STDLIB_H )
check_include_files( "stdlib.h" HAVE_STDLIB_H )
check_include_files( "string.h" HAVE_STRING_H )
check_include_files( "strings.h" HAVE_STRINGS_H )
check_include_files( "sys/resource.h" HAVE_SYS_WAIT_H )
check_include_files( "sys/stat.h" HAVE_SYS_STAT_H )
check_include_files( "sys/time.h" HAVE_SYS_TIME_H )
check_include_files( "sys/times.h" HAVE_SYS_TIMES_H )
check_include_files( "sys/types.h" HAVE_SYS_TYPES_H )
check_include_files( "sys/wait.h" HAVE_SYS_WAIT_H )
check_include_files( "unistd.h" HAVE_UNISTD_H )

include( CheckIncludeFileCXX )
check_include_file_cxx( "algorithm" HAVE_ALGORITHM )
check_include_file_cxx( "bitset" HAVE_BITSET )
check_include_file_cxx( "cassert" HAVE_CASSERT )
check_include_file_cxx( "cctype" HAVE_CCTYPE )
check_include_file_cxx( "cerrno" HAVE_CERRNO )
check_include_file_cxx( "cfloat" HAVE_CFLOAT )
check_include_file_cxx( "climits" HAVE_CLIMITS )
check_include_file_cxx( "cmath" HAVE_CMATH )
check_include_file_cxx( "cstddef" HAVE_CSTDDEF )
check_include_file_cxx( "cstdlib" HAVE_CSTDLIB )
check_include_file_cxx( "cstring" HAVE_CSTRING )
check_include_file_cxx( "ctime" HAVE_CTIME )
check_include_file_cxx( "deque" HAVE_DEQUE )
check_include_file_cxx( "fstream" HAVE_FSTREAM )
check_include_file_cxx( "functional" HAVE_FUNCTIONAL )
check_include_file_cxx( "iomanip" HAVE_IOMANIP )
check_include_file_cxx( "iosfwd" HAVE_IOSFWD )
check_include_file_cxx( "iostream" HAVE_IOSTREAM )
check_include_file_cxx( "istream" HAVE_ISTREAM )
check_include_file_cxx( "iterator" HAVE_ITERATOR )
check_include_file_cxx( "limits" HAVE_LIMITS )
check_include_file_cxx( "list" HAVE_LIST )
check_include_file_cxx( "map" HAVE_MAP )
check_include_file_cxx( "memory" HAVE_MEMORY )
check_include_file_cxx( "new" HAVE_NEW )
check_include_file_cxx( "numeric" HAVE_NUMERIC )
check_include_file_cxx( "ostream" HAVE_OSTREAM )
check_include_file_cxx( "queue" HAVE_QUEUE )
check_include_file_cxx( "set" HAVE_SET )
check_include_file_cxx( "sstream" HAVE_SSTREAM )
check_include_file_cxx( "stdexcept" HAVE_STDEXCEPT )
check_include_file_cxx( "streambuf" HAVE_STREAMBUF )
check_include_file_cxx( "string" HAVE_STRING )
check_include_file_cxx( "strstream" HAVE_STRSTREAM )
check_include_file_cxx( "typeinfo" HAVE_TYPEINFO )
check_include_file_cxx( "utility" HAVE_UTILITY )
check_include_file_cxx( "vector" HAVE_VECTOR )


# Check types exist
include( CheckTypeSize )
check_type_size( "long long" LONG_LONG_SIZE ) # also sets HAVE_LONG_LONG
if ( LONG_LONG_SIZE GREATER 0 )
  set( HAVE_LONG_LONG ON )
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
check_function_exists( fork "unistd.h" HAVE_FORK )
check_function_exists( execvp "unistd.h" HAVE_EXECVP )
check_function_exists( opendir "dirent.h" HAVE_OPENDIR )

################################################################################
##################         All User Defined options           ##################
################################################################################

# will be set in toolchain file
set( ENABLE_BLUEGENE OFF CACHE STRING "Configure for BlueGene." )

option( K_COMPUTER "Enable K computer." OFF )
set( TICS_PER_MS "1000.0" CACHE STRING "Specify elementary unit of time. [default 1000.0]" )
set( TICS_PER_STEP "100" CACHE STRING "Specify resolution. [default 100]" )
option( WITH_MPI "Request compilation with MPI; optionally give directory with MPI installation." OFF )
option( LIBNEUROSIM "Request the use of libneurosim; optionally give the directory, where libneurosim is installed." OFF )
set( WITH_OPTIMIZE OFF CACHE STRING "Enable user defined optimizations. [default OFF, when ON, '-O3']" )
set( WITH_WARNING ON CACHE STRING "Enable user defined warnings. [default ON, when ON '-Wall']" )
set( WITH_DEBUG OFF CACHE STRING "Enable user defined debug flags. [default OFF, when ON '-g']" )
option( WITH_GSL "Find a gsl library [default=ON]. To set a specific gsl installation, set install path." ON)
option( WITH_READLINE "Find a readline library [default=ON]. To set a specific readline, set install path." ON )
option( WITH_OPENMP "Enable OpenMP multithreading [default=ON]. Optional: set OMP flag." ON )
option( WITH_PYTHON "Find a Python interpreter [default=ON]. To ignore the Python set to OFF. To set a specific Python, set path." ON )
option( EXTERNAL_MODULES "External modules to be linked in. Devided by ; ." OFF )
option( STATIC_LIBRARIES "Build static libraries. [default=no]" OFF )
option( PS_ARRAYS "Use PS array construction semantics" ON )
option( WITH_MUSIC "Request the use of MUSIC; optionally give the directory, where MUSIC is installed." OFF )

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
if ( WITH_OPTIMIZE )
  if ( WITH_OPTIMIZE STREQUAL "ON" )
    set( WITH_OPTIMIZE "-O3" )
  endif ()
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${WITH_OPTIMIZE}" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WITH_OPTIMIZE}" )
endif ( WITH_OPTIMIZE )

if ( WITH_DEBUG )
  if ( WITH_DEBUG STREQUAL "ON" )
    set( WITH_DEBUG "-g" )
  endif ()
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${WITH_DEBUG}" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WITH_DEBUG}" )
endif ( WITH_DEBUG )

if ( WITH_WARNING )
  if ( WITH_WARNING STREQUAL "ON" )
    set( WITH_WARNING "-Wall" )
  endif ()
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${WITH_WARNING}" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WITH_WARNING}" )
endif ( WITH_WARNING )

if ( K_COMPUTER )
  # TODO use the -DCMAKE_TOOLCHAIN_FILE= argument
  set( IS_K ON )
endif ( K_COMPUTER )

# is set in the BlueGeneQ.cmake file
if ( ENABLE_BLUEGENE )
  if ( ${ENABLE_BLUEGENE} STREQUAL "L" )
    set( IS_BLUEGENE_L ON )
  elseif ( ${ENABLE_BLUEGENE} STREQUAL "P" )
    set( IS_BLUEGENE_P ON )
  elseif ( ${ENABLE_BLUEGENE} STREQUAL "Q" )
    set( IS_BLUEGENE_Q ON )
  else ()
    message( FATAL_ERROR "Only L/P/Q is allowed for ENABLE_BLUEGENE." )
  endif ()
  set( IS_BLUEGENE ON )
else ()
  set( IS_BLUEGENE OFF )
endif ( ENABLE_BLUEGENE )

# build static or shared libraries
if ( STATIC_LIBRARIES )
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
if ( LIBNEUROSIM )
  set( SLI_MODULES ${SLI_MODULES} conngen )
endif ( LIBNEUROSIM )

set( SLI_MODULE_INCLUDE_DIRS )
foreach ( mod ${SLI_MODULES} )
  list( APPEND SLI_MODULE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/${mod}" )
endforeach ()

if ( EXTERNAL_MODULES )
  # headers from external modules will be installed here
  include_directories( "${CMAKE_INSTALL_PREFIX}/${INSTALL_INC_DIR}" )

  # put all external libs into this variable
  set( EXTERNAL_MODULE_LIBRARIES )
  # put all external headers into this variable
  set( EXTERNAL_MODULE_INCLUDES )
  foreach ( mod ${EXTERNAL_MODULES} )
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
endif ( EXTERNAL_MODULES )


# write static_modules.h
file( WRITE ${PROJECT_BINARY_DIR}/nest/static_modules.h "#ifndef STATIC_MODULES_H\n" )
file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#define STATIC_MODULES_H\n\n" )
file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "// Add all in source modules:\n" )
foreach ( mod ${SLI_MODULES} )
  file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#include \"${mod}module.h\"\n" )
endforeach ( mod )

# when we build statically, we need to add headers and addmodule for external modules
# just as if it were a in source module.
if ( STATIC_LIBRARIES AND EXTERNAL_MODULES )
  file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "\n// Add all external modules:\n" )
  foreach ( mod ${EXTERNAL_MODULES} )
    file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "#include \"${mod}module.h\"\n" )
  endforeach ( mod )
endif ( STATIC_LIBRARIES AND EXTERNAL_MODULES )
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
if ( STATIC_LIBRARIES AND EXTERNAL_MODULES )
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
endif ( STATIC_LIBRARIES AND EXTERNAL_MODULES )

file( APPEND ${PROJECT_BINARY_DIR}/nest/static_modules.h "}\n\n#endif\n" )
# write static_modules.h

# Set tics per ms / step
if ( TICS_PER_MS )
  set( HAVE_TICS_PER_MS ON )
endif ( TICS_PER_MS )

if ( TICS_PER_STEP )
  set( HAVE_TICS_PER_STEP ON )
endif ( TICS_PER_STEP )

################################################################################
##################            Find Libraries here             ##################
################################################################################

# Only find libLTDL if we link dynamically
if ( NOT STATIC_LIBRARIES )
  find_package( LTDL )
  if ( LTDL_FOUND )
    set( HAVE_LIBLTDL ON )

    include_directories( ${LTDL_INCLUDE_DIR} )
    link_libraries( ${LTDL_LIBRARIES} )
  else ()
    set( HAVE_LIBLTDL OFF )
  endif ( LTDL_FOUND )
else ()
  set( HAVE_LIBLTDL OFF )
endif ()

# GSL finding
if ( WITH_GSL )
  if ( NOT ${WITH_GSL} STREQUAL "ON" )
    # if set, use this prefix
    set( GSL_ROOT_DIR ${WITH_GSL} )
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
  elseif ( GSL_FOUND )
    set( HAVE_GSL OFF )
    set( HAVE_GSL_1_2 OFF )
    set( HAVE_GSL_1_11 OFF )
  endif ( GSL_FOUND )
endif ( WITH_GSL )

# libneurosim
if ( LIBNEUROSIM )
  if ( NOT ${LIBNEUROSIM} STREQUAL "ON" )
    # a path is set
    set( LIBNEUROSIM_ROOT ${LIBNEUROSIM} )
  endif ()

  find_package( LibNeurosim )
  if ( LIBNEUROSIM_FOUND )
    set( HAVE_LIBNEUROSIM ON )

    include_directories( ${LIBNEUROSIM_INCLUDE_DIR} )
    link_libraries( ${LIBNEUROSIM_LIBRARIES} )
  endif ( LIBNEUROSIM_FOUND )
endif ( LIBNEUROSIM )

# Python executable finding
if ( WITH_PYTHON )
  if ( NOT ${WITH_PYTHON} STREQUAL "ON" )
    # a path is set
    set( PYTHON_EXECUTABLE ${WITH_PYTHON} )
  endif ()

  find_package( PythonInterp )
  if ( PYTHONINTERP_FOUND )
    set( PYTHON ${PYTHON_EXECUTABLE} )
    set( PYTHON_VERSION ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} )

    # need python lib and header...
    find_package( PythonLibs )
    if ( PYTHONLIBS_FOUND )
      set( HAVE_PYTHON ON )
    endif ()
  endif ()

  find_package( Cython )

  set( PYEXECDIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages )
else ( WITH_PYTHON )
  set( HAVE_PYTHON OFF )
endif ( WITH_PYTHON )

# Find readline
if ( WITH_READLINE )
  if ( NOT ${WITH_READLINE} STREQUAL "ON" )
    # a path is set
    set( READLINE_ROOT_DIR ${WITH_READLINE} )
  endif ()

  find_package( Readline )
  if ( READLINE_FOUND )
    set( HAVE_READLINE ON )
    include_directories( ${READLINE_INCLUDE_DIRS} )
    link_libraries( ${READLINE_LIBRARIES} )
  else ( READLINE_FOUND )
    set( HAVE_READLINE OFF )
  endif ( READLINE_FOUND )
else ()
  set( HAVE_READLINE OFF )
endif ()

# OPENMP finding
if ( WITH_OPENMP )
  if ( NOT ${WITH_OPENMP} STREQUAL "ON" )
    # a path is set
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${WITH_OPENMP}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WITH_OPENMP}" )
  else ()
    find_package( OpenMP )
    if ( OPENMP_FOUND )
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${OpenMP_C_FLAGS}" )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
    endif ( OPENMP_FOUND )
  endif ()
endif ( WITH_OPENMP )

# MPI finding
if ( WITH_MPI )
  find_package( MPI )
  if ( MPI_CXX_FOUND )
    set( HAVE_MPI ON )

    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${MPI_C_COMPILE_FLAGS}" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}" )

    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}" )
    include_directories( ${MPI_CXX_INCLUDE_PATH} )
    link_libraries( ${MPI_CXX_LIBRARIES} )
  else ( MPI_CXX_FOUND )
    set( HAVE_MPI OFF )
  endif ( MPI_CXX_FOUND )
else ()
  set( HAVE_MPI OFF )
endif ()

if ( WITH_MUSIC )
  if ( NOT ${WITH_MUSIC} STREQUAL "ON" )
    # a path is set
    set( MUSIC_ROOT_DIR "${WITH_MUSIC}" )
  endif ()

  if ( NOT HAVE_MPI )
    message( FATAL_ERROR "MUSIC requires -DWITH_MPI=ON." )
  endif ( NOT HAVE_MPI )

  find_package( Music )
  include_directories( ${MUSIC_INCLUDE_DIR} )
  link_libraries( ${MUSIC_LIBRARY} )
  if ( MUSIC_FOUND )

    set( HAVE_MUSIC ON )
  endif ( MUSIC_FOUND )
else ( WITH_MUSIC )
  set( HAVE_MUSIC OFF )
endif ( WITH_MUSIC )

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

# all libraries
set( ALL_LIBS "${OpenMP_CXX_FLAGS};${LTDL_LIBRARIES};${READLINE_LIBRARIES};${GSL_LIBRARIES};${LIBNEUROSIM_LIBRARIES};${MUSIC_LIBRARY};${MPI_CXX_LIBRARIES}" )
string( REPLACE ";" " " ALL_LIBS "${ALL_LIBS}" )

# all includes
set( ALL_INCLUDES_tmp "${LTDL_INCLUDE_DIR};${READLINE_INCLUDE_DIRS};${GSL_INCLUDE_DIRS};${LIBNEUROSIM_INCLUDE_DIR};${MUSIC_INCLUDE_DIR};${MPI_CXX_INCLUDE_PATH}" )
set( ALL_INCLUDES "" )
foreach ( INC ${ALL_INCLUDES_tmp} )
  if ( NOT INC STREQUAL "" )
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
    ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
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
if ( EXTERNAL_MODULES )
  message( "User modules        : ${EXTERNAL_MODULES}" )
  foreach ( mod ${EXTERNAL_MODULES} )
    message( "  ${mod}:" )
    message( "    Header          : ${${mod}_EXT_MOD_INCLUDE}" )
    message( "    Library         : ${${mod}_EXT_MOD_LIBRARY}" )
    message( "" )
  endforeach ()
else ( EXTERNAL_MODULES )
  message( "User modules        : None" )
endif ( EXTERNAL_MODULES )

if ( HAVE_PYTHON )
  message( "Python bindings     : Yes (Python ${PYTHON_VERSION}: ${PYTHON})" )
  message( "Cython bindings     : Yes (Cython: ${CYTHON_EXECUTABLE})" )
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
  message( "Use Readline        : Yes" )
  message( "    Includes        : ${READLINE_INCLUDE_DIRS}" )
  message( "    Libraries       : ${READLINE_LIBRARIES}" )
  message( "" )
else ( HAVE_READLINE )
  message( "Use Readline        : No" )
endif ( HAVE_READLINE )

if ( HAVE_LIBLTDL )
  message( "Use libltdl         : Yes" )
  message( "    Includes        : ${LTDL_INCLUDE_DIR}" )
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
  message( "Use MUSIC           : Yes" )
  message( "    Includes        : ${MUSIC_INCLUDE_DIR}" )
  message( "    Libraries       : ${MUSIC_LIBRARY}" )
  message( "" )
else ( HAVE_MUSIC )
  message( "Use MUSIC           : No" )
endif ( HAVE_MUSIC )

if ( HAVE_LIBNEUROSIM )
  message( "Use libneurosim     : Yes" )
  message( "    Includes        : ${LIBNEUROSIM_INCLUDE_DIR}" )
  message( "    Libraries       : ${LIBNEUROSIM_LIBRARIES}" )
  message( "" )
else ( HAVE_LIBNEUROSIM )
  message( "Use libneurosim     : No" )
endif ( HAVE_LIBNEUROSIM )

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
