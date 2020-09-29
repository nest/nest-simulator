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
# along with NEST.  If not, see <http://www.gnu.org/licenses/>

# Version range from minimum required (3.12 for macOS OpenMP)
# up to newest version tested, see https://cliutils.gitlab.io/modern-cmake/chapters/basics.html
cmake_minimum_required( VERSION 3.12...3.16 )

# add cmake modules: for all `include(...)` first look here
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

project( nest CXX C )
set( NEST_USER_EMAIL "users@nest-simulator.org" )

################################################################################
##################         All User Defined options           ##################
################################################################################

# set NEST defaults
set( tics_per_ms "1000.0" CACHE STRING "Specify elementary unit of time. [default 1000.0]" )
set( tics_per_step "100" CACHE STRING "Specify resolution. [default 100]" )
set( connector_cutoff "3" CACHE STRING "Specify when to truncate the recursive instantiation of the connector. [default 3]" )
option( with-ps-arrays "Use PS array construction semantics. [default=ON]" ON )

# add user modules
set( external-modules OFF CACHE STRING "External NEST modules to be linked in, separated by ';'. [default=OFF]" )

# connect NEST with external projects
set( with-libneurosim OFF CACHE STRING "Request the use of libneurosim. Optionally give the directory, where libneurosim is installed. [default=OFF]" )
set( with-music OFF CACHE STRING "Request the use of MUSIC. Optionally give the directory, where MUSIC is installed. [default=OFF]" )
set( with-sionlib OFF CACHE STRING "Request the use of SIONlib. Optionally give the directory where SIONlib is installed. [default=OFF]" )
set( with-recordingbackend-arbor OFF CACHE STRING "Request compilation of the recording backend for Arbor. Requires MPI, Python and mpi4py. [default=OFF]" )

# set parallelization scheme
set( with-mpi OFF CACHE STRING "Request compilation with MPI. Optionally give directory with MPI installation. [default=OFF]" )
set( with-openmp ON CACHE BOOL "Enable OpenMP multithreading. Optional: set OMP flag. [default=ON]" )

# define default libraries
set( with-gsl ON CACHE STRING "Find a gsl library. To set a specific gsl installation, set install path. [default=ON]" )
set( with-readline ON CACHE STRING "Find a readline library. To set a specific readline, set install path. [default=ON]" )
set( with-ltdl ON CACHE STRING "Find a ltdl library. To set a specific ltdl, set install path. [default=ON]" )
set( with-python ON CACHE STRING "Build PyNEST. To set a specific Python, set install path. [default=ON]" )
option( cythonize-pynest "Use Cython to cythonize pynestkernel.pyx. If OFF, PyNEST has to be build from a pre-cythonized pynestkernel.pyx. [default=ON]" ON )
set( with-boost ON CACHE STRING "Find a Boost library. To set a specific Boost installation, set install path. [default=ON]" )

# Whether to build a 'mostly' static executable and static libraries.
option( static-libraries "Build static executable and libraries. [default=OFF]" OFF )
# additional compile flags
set( with-optimize ON CACHE STRING "Enable user defined optimizations. [default ON, when ON, defaults to '-O2']" )
set( with-warning ON CACHE STRING "Enable user defined warnings. [default ON, when ON, defaults to '-Wall']" )
set( with-debug OFF CACHE STRING "Enable user defined debug flags. [default OFF, when ON, defaults to '-g']" )
set( with-intel-compiler-flags OFF CACHE STRING "User defined flags for the Intel compiler. [defaults to '-fp-model strict']" )
set( with-libraries OFF CACHE STRING "Link additional libraries. Give full path. Separate multiple libraries by ';'. [default OFF]" )
set( with-includes OFF CACHE STRING "Add additional include paths. Give full path without '-I'. Separate multiple include paths by ';'. [default OFF]" )
set( with-defines OFF CACHE STRING "Additional defines, e.g. '-DXYZ=1'. Separate multiple defines by ';'. [default OFF]" )
set( with-version-suffix "" CACHE STRING "Set a user defined version suffix. [default '']" )

# cross-compiling
# should be set via toolchain files
set( enable-bluegene OFF CACHE STRING "Configure for BlueGene." )
option( k-computer "Enable K computer." OFF )

set( target-bits-split "standard" CACHE STRING "Split of the 64-bit target neuron identifier type. 'standard' is recommended for most users. If running on more than 262144 MPI processes or more than 512 threads, change to 'hpc'. [default standard]" )

################################################################################
##################      Project Directory variables           ##################
################################################################################

# In general use the CMAKE_INSTALL_<dir> and CMAKE_INSTALL_FULL_<dir> vars from
# GNUInstallDirs, but the CMAKE_INSTALL_DATADIR is usually just CMAKE_INSTALL_DATAROOTDIR
# and we want it to be CMAKE_INSTALL_DATAROOTDIR/PROJECT_NAME
set( CMAKE_INSTALL_DATADIR "share/${PROJECT_NAME}" CACHE STRING "Relative directory, where NEST installs its data (share/nest)" )

include( GNUInstallDirs )

################################################################################
##################           Find utility programs            ##################
################################################################################

# needed for pynest test suite
if ( ${with-python} STREQUAL "ON" )
  find_program( NOSETESTS NAMES nosetests )
endif ()

# needed for target doc and fulldoc
find_package( Doxygen )
find_program( SED NAMES sed gsed )

################################################################################
##################                Load includes               ##################
################################################################################

# This include checks the symbols, etc.
include( CheckIncludesSymbols )

# These includes publish function names.
include( ProcessOptions )
include( WriteStaticModules_h )
include( CheckExtraCompilerFeatures )
include( ConfigureSummary )
include( GetTriple )
include( DefaultCompilerFlags )

# get triples arch-vendor-os
get_host_triple( NEST_HOST_TRIPLE NEST_HOST_ARCH NEST_HOST_VENDOR NEST_HOST_OS )
get_target_triple( NEST_TARGET_TRIPLE NEST_TARGET_ARCH NEST_TARGET_VENDOR NEST_TARGET_OS )


# set default compiler flags
nest_set_default_compiler_flags()

# Process the command line arguments
nest_process_with_optimize()
nest_process_with_debug()
nest_process_with_intel_compiler_flags()
nest_process_with_warning()
nest_process_with_libraries()
nest_process_with_includes()
nest_process_with_defines()
nest_process_k_computer()
nest_process_enable_bluegene()
nest_process_static_libraries()
nest_process_external_modules()
nest_process_tics_per_ms()
nest_process_tics_per_step()
nest_process_with_ps_array()
nest_process_with_libltdl()
nest_process_with_readline()
nest_process_with_gsl()
nest_process_with_python()
nest_process_with_openmp()
nest_process_with_mpi()
nest_process_with_libneurosim()
nest_process_with_music()
nest_process_with_sionlib()
nest_process_with_mpi4py()
nest_process_with_boost()
nest_process_with_recordingbackend_arbor()
nest_process_target_bits_split()
nest_process_version_suffix()

nest_get_color_flags()
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NEST_C_COLOR_FLAGS}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NEST_CXX_COLOR_FLAGS}" )

# requires HAVE_LIBNEUROSIM
nest_default_modules()

nest_write_static_module_header( "${PROJECT_BINARY_DIR}/nest/static_modules.h" )

# check additionals
nest_check_exitcode_abort()
nest_check_exitcode_segfault()
nest_check_have_cmath_makros_ignored()
nest_check_have_alpha_cxx_std_bug()
nest_check_have_sigusr_ignored()
nest_check_have_static_template_declaration_fail()
nest_check_have_stl_vector_capacity_base_unity()
nest_check_have_stl_vector_capacity_doubling()
nest_check_have_xlc_ice_on_using()
nest_check_have_std_nan()
nest_check_have_std_isnan()

################################################################################
##################           Create version string            ##################
################################################################################

include( NestVersionInfo )
get_version_info()
message("-- NEST version: ${NEST_VERSION_STRING}")

################################################################################
##################         Enable Testing Targets             ##################
################################################################################
enable_testing()
set( TEST_OPTS "" )

if ( HAVE_PYTHON )
  set( TEST_OPTS "${TEST_OPTS};--with-python=${PYTHON}" )
endif ()

if ( HAVE_MUSIC )
  set( TEST_OPTS "${TEST_OPTS};--with-music=${MUSIC_EXECUTABLE}" )
endif ()

add_custom_target( installcheck
  COMMAND ${CMAKE_COMMAND} -E env
    ${CMAKE_INSTALL_FULL_DATADIR}/extras/do_tests.sh
	--prefix=${CMAKE_INSTALL_PREFIX}
	--report-dir="${PROJECT_BINARY_DIR}/reports"
	${TEST_OPTS}
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMENT "Executing NEST's testsuite..."
)

# N.B. to ensure "make install" is always run before "make installcheck", we
# would ideally like to add:
#   add_dependencies( installcheck install )
# However, an issue in CMake at time of writing (May 2020, see
# https://gitlab.kitware.com/cmake/cmake/-/issues/8438) precludes us from doing
# so.

################################################################################
##################        Define Subdirectories here          ##################
################################################################################

if ( HAVE_LIBNEUROSIM )
  add_subdirectory( conngen )
endif ()
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
add_subdirectory( testsuite )
if ( HAVE_PYTHON )
  add_subdirectory( pynest )
endif ()

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
  foreach ( def ${with-defines} )
    set( ALL_CFLAGS "${def} ${ALL_CFLAGS}" )
    set( ALL_CXXFLAGS "${def} ${ALL_CXXFLAGS}" )
  endforeach ()
endif ()
# add sionlib defines
foreach ( def ${SIONLIB_DEFINES} )
    set( ALL_CFLAGS "${ALL_CFLAGS} ${def}" )
    set( ALL_CXXFLAGS "${ALL_CXXFLAGS} ${def}" )
endforeach ()

# all libraries
set( ALL_LIBS
  "-lnestutil"
  "-lnest"
  "-lrandom"
  "-lsli"
  "-lnestkernel"
  "${OpenMP_CXX_FLAGS}"
  "${LTDL_LIBRARIES}"
  "${READLINE_LIBRARIES}"
  "${GSL_LIBRARIES}"
  "${LIBNEUROSIM_LIBRARIES}"
  "${MUSIC_LIBRARIES}"
  "${MPI_CXX_LIBRARIES}"
  "${SIONLIB_LIBRARIES}"
  "${BOOST_LIBRARIES}" )

if ( with-libraries )
  set( ALL_LIBS "${ALL_LIBS};${with-libraries}" )
endif ()
string( REPLACE ";" " " ALL_LIBS "${ALL_LIBS}" )

# all includes
set( ALL_INCLUDES_tmp
  "${CMAKE_INSTALL_FULL_INCLUDEDIR}/nest"
  "${LTDL_INCLUDE_DIRS}"
  "${READLINE_INCLUDE_DIRS}"
  "${GSL_INCLUDE_DIRS}"
  "${LIBNEUROSIM_INCLUDE_DIRS}"
  "${MUSIC_INCLUDE_DIRS}"
  "${MPI_CXX_INCLUDE_PATH}"
  "${BOOST_INCLUDE_DIR}" )
set( ALL_INCLUDES "" )
foreach ( INC ${ALL_INCLUDES_tmp} ${with-includes} )
  if ( INC AND NOT INC STREQUAL "" )
    set( ALL_INCLUDES "${ALL_INCLUDES} -I${INC}" )
  endif ()
endforeach ()
set( ALL_INCLUDES "${ALL_INCLUDES} ${SIONLIB_INCLUDE}" )

if ( HAVE_LIBNEUROSIM )
  set( ALL_LIBS "-lconngen ${ALL_LIBS}" )
  set( ALL_INCLUDES "-I${PROJECT_SOURCE_DIR}/conngen ${ALL_INCLUDES}" )
endif ()

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
    "${PROJECT_SOURCE_DIR}/extras/nest-config.in"
    "${PROJECT_BINARY_DIR}/extras/nest-config" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/extras/nest_vars.sh.in"
    "${PROJECT_BINARY_DIR}/extras/nest_vars.sh" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/doc/normaldoc.conf.in"
    "${PROJECT_BINARY_DIR}/doc/normaldoc.conf" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/doc/fulldoc.conf.in"
    "${PROJECT_BINARY_DIR}/doc/fulldoc.conf" @ONLY
)


################################################################################
##################            Install Extra Files             ##################
################################################################################

install( FILES LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )

add_custom_target( install-nodoc
  COMMAND make NEST_INSTALL_NODOC=true install
)

nest_print_config_summary()
