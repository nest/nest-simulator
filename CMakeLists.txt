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

cmake_minimum_required( VERSION 3.19 )

# add cmake modules: for all `include(...)` first look here
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

project( nest CXX C )
set( NEST_USER_EMAIL "users@nest-simulator.org" )

include( ColorMessages )

# check if the given CMAKE_INSTALL_PREFIX is not empty
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "")
  printError("CMAKE_INSTALL_PREFIX cannot be an empty string")
endif()

# handle relative installation prefixes
if( NOT IS_ABSOLUTE ${CMAKE_INSTALL_PREFIX})
    # convert relative path to absolute path
    get_filename_component(absPath ${CMAKE_INSTALL_PREFIX} ABSOLUTE BASE_DIR ${CMAKE_BINARY_DIR})
    set(CMAKE_INSTALL_PREFIX ${absPath})
    printInfo("Relative CMAKE_INSTALL_PREFIX has been converted to absolute path ${CMAKE_INSTALL_PREFIX}")
endif()

################################################################################
##################         All User Defined options           ##################
################################################################################

# use Python to build PyNEST
set( with-python ON CACHE STRING "Build PyNEST [default=ON]." )
option( cythonize-pynest "Use Cython to cythonize pynestkernel.pyx [default=ON]. If OFF, PyNEST has to be build from a pre-cythonized pynestkernel.pyx." ON )

# select parallelization scheme
set( with-mpi OFF CACHE STRING "Build with MPI parallelization [default=OFF]." )
set( with-openmp ON CACHE BOOL "Build with OpenMP multi-threading [default=ON]. Optionally set OMP compiler flags." )

# external libraries
set( with-libneurosim OFF CACHE STRING "Build with libneurosim [default=OFF]. Optionally give the directory where libneurosim is installed." )
set( with-music OFF CACHE STRING "Build with MUSIC [default=OFF]. Optionally give the directory where MUSIC is installed." )
set( with-sionlib OFF CACHE STRING "Build with SIONlib [default=OFF]. Optionally give the directory where sionlib is installed." )
set( with-boost ON CACHE STRING "Build with Boost [default=ON]. To set a specific Boost installation, give the install path." )
set( with-hdf5 OFF CACHE STRING "Find a HDF5 library. To set a specific HDF5 installation, set install path. [default=ON]" )
set( with-readline ON CACHE STRING "Build with GNU Readline library [default=ON]. To set a specific library, give the install path." )
set( with-ltdl ON CACHE STRING "Build with ltdl library [default=ON]. To set a specific ltdl, give the  install path. NEST uses ltdl for dynamic loading of external user modules." )
set( with-gsl ON CACHE STRING "Build with the GSL library [default=ON]. To set a specific library, give the install path." )

# NEST properties
set( with-modelset "full" CACHE STRING "The modelset to include. Sample configurations are in the modelsets directory. This option is mutually exclusive with -Dwith-models. [default=full]." )
set( with-models OFF CACHE STRING "The models to include as a semicolon-separated list of model headers (without the .h extension). This option is mutually exclusive with -Dwith-modelset. [default=OFF]." )
set( tics_per_ms "1000.0" CACHE STRING "Specify elementary unit of time [default=1000 tics per ms]." )
set( tics_per_step "100" CACHE STRING "Specify resolution [default=100 tics per step]." )
set( with-detailed-timers OFF CACHE STRING "Build with detailed internal time measurements [default=OFF]. Detailed timers can affect the performance." )
set( with-mpi-sync-timer OFF CACHE STRING "Build with mpi synchronization barrier and timer [default=OFF]. Can affect the performance." )
set( with-threaded-timers ON CACHE STRING "Build with one internal timer per thread [default=ON]. Multi-threaded timers can affect the performance." )
set( target-bits-split "standard" CACHE STRING "Split of the 64-bit target neuron identifier type [default='standard']. 'standard' is recommended for most users. If running on more than 262144 MPI processes or more than 512 threads, change to 'hpc'." )

# generic build configuration
option( static-libraries "Build static executable and libraries [default=OFF]" OFF )
set( with-optimize ON CACHE STRING "Enable user defined optimizations [default=ON (uses '-O2')]. When OFF, no '-O' flag is passed to the compiler. Explicit compiler flags can be given; separate multiple flags by ';'." )
set( with-warning ON CACHE STRING "Enable user defined warnings [default=ON (uses '-Wall')]. Separate  multiple flags by ';'." )
set( with-debug OFF CACHE STRING "Enable user defined debug flags [default=OFF]. When ON, '-g' is used. Separate  multiple flags by ';'." )
set( with-cpp-std "c++17" CACHE STRING "C++ standard to use for compilation [default='c++17']." )
set( with-intel-compiler-flags OFF CACHE STRING "User defined flags for the Intel compiler [default='-fp-model strict']. Separate multiple flags by ';'." )
set( with-libraries OFF CACHE STRING "Link additional libraries [default=OFF]. Give full path. Separate multiple libraries by ';'." )
set( with-includes OFF CACHE STRING "Add additional include paths [default=OFF]. Give full path without '-I'. Separate multiple include paths by ';'." )
set( with-defines OFF CACHE STRING "Additional defines, e.g. '-DXYZ=1' [default=OFF]. Separate multiple defines by ';'." )

# documentation build configuration
set( with-userdoc OFF CACHE STRING "Build user documentation [default=OFF]")
set( with-devdoc OFF CACHE STRING "Build developer documentation [default=OFF]")

set( with-full-logging OFF CACHE STRING "Write debug output to 'dump_<num_ranks>_<rank>.log' file [default=OFF]")

################################################################################
##################      Project Directory variables           ##################
################################################################################

# In general use the CMAKE_INSTALL_<dir> and CMAKE_INSTALL_FULL_<dir> vars from
# GNUInstallDirs (included after calling nest_process_with_python()), but the
# CMAKE_INSTALL_DATADIR is usually just CMAKE_INSTALL_DATAROOTDIR
# and we want it to be CMAKE_INSTALL_DATAROOTDIR/PROJECT_NAME
set( CMAKE_INSTALL_DATADIR "share/${PROJECT_NAME}" CACHE STRING "Relative directory, where NEST installs its data (share/nest)" )

################################################################################
##################           Find utility programs            ##################
################################################################################

find_program( SED NAMES sed gsed )

################################################################################
##################                Load includes               ##################
################################################################################

# This include checks the symbols, etc.
include( CheckIncludesSymbols )

# These includes publish function names.
include( ProcessOptions )
include( CheckExtraCompilerFeatures )
include( ConfigureSummary )
include( GetTriple )

# enable link-time optimizations
include(CheckIPOSupported)
check_ipo_supported()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# --- PGO Configuration Start ---
set(PGO_MODE "OFF" CACHE STRING "Control Profile Guided Optimization: OFF, GENERATE, USE")

if(PGO_MODE STREQUAL "GENERATE")
    message(STATUS "PGO: Instrumentation Build Enabled")
    add_compile_options(-fprofile-generate)
    add_link_options(-fprofile-generate)

elseif(PGO_MODE STREQUAL "USE")
    message(STATUS "PGO: Optimized Build Enabled")
    add_compile_options(-fprofile-use -fprofile-correction)
    add_link_options(-fprofile-use -fprofile-correction)
endif()
# --- PGO Configuration End ---

# get triples arch-vendor-os
get_host_triple( NEST_HOST_TRIPLE NEST_HOST_ARCH NEST_HOST_VENDOR NEST_HOST_OS )
get_target_triple( NEST_TARGET_TRIPLE NEST_TARGET_ARCH NEST_TARGET_VENDOR NEST_TARGET_OS )

# Process the command line arguments
# IMPORTANT: Do not change the order of nest_process_with_python() and include( GNUInstallDirs )!
#            If NEST is built with Python, nest_process_with_python() defaults CMAKE_INSTALL_PREFIX
#            to the active virtual Python environment. This effects the inclusion
#            of GNUInstallDirs defining CMAKE_INSTALL_<dir> and CMAKE_INSTALL_FULL_<dir>.
nest_process_with_python()
include( GNUInstallDirs )
nest_post_process_with_python()
nest_process_with_std()
nest_process_with_intel_compiler_flags()
nest_process_with_warning()
nest_process_with_libraries()
nest_process_with_includes()
nest_process_with_defines()
nest_process_static_libraries()
nest_process_tics_per_ms()
nest_process_tics_per_step()
nest_process_with_libltdl()
nest_process_with_readline()
nest_process_with_gsl()
nest_process_with_openmp()
nest_process_with_mpi()
nest_process_with_detailed_timers()
nest_process_with_threaded_timers()
nest_process_with_mpi_sync_timer()
nest_process_with_libneurosim()
nest_process_with_music()
nest_process_with_sionlib()
nest_process_with_mpi4py()
nest_process_with_boost()
nest_process_with_hdf5()
nest_process_target_bits_split()
nest_process_userdoc()
nest_process_devdoc()
nest_process_full_logging()

nest_process_models()

# These two function calls must come last, as to prevent unwanted interactions of the newly set flags
# with detection/compilation operations carried out in earlier functions. The optimize/debug flags set
# using these functions should only apply to the compilation of NEST, not to that of test programs
# generated by CMake when it tries to detect compiler options or such.
nest_process_with_optimize()
nest_process_with_debug()

nest_get_color_flags()
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NEST_C_COLOR_FLAGS}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NEST_CXX_COLOR_FLAGS}" )

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
nest_check_random123()

include( NestVersionInfo )
get_version_info()
printInfo("Done configuring NEST version: ${NEST_VERSION}")

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
    ${CMAKE_INSTALL_FULL_DATADIR}/testsuite/do_tests.sh
	--prefix=${CMAKE_INSTALL_PREFIX}
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

add_subdirectory( doc )
add_subdirectory( bin )
add_subdirectory( examples )
add_subdirectory( build_support )
add_subdirectory( lib )
add_subdirectory( libnestutil )
add_subdirectory( models )
add_subdirectory( sli )
add_subdirectory( nest )
add_subdirectory( nestkernel )
add_subdirectory( thirdparty )
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
  printError( "Unknown build type: '${CMAKE_BUILD_TYPE}'" )
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

# libraries required to link extension modules
set( MODULE_LINK_LIBS
  "-lnest"
  "-lsli"
  "${LTDL_LIBRARIES}"
  "${READLINE_LIBRARIES}"
  "${GSL_LIBRARIES}"
  "${LIBNEUROSIM_LIBRARIES}"
  "${MUSIC_LIBRARIES}"
  "${MPI_CXX_LIBRARIES}"
  "${OpenMP_CXX_LIBRARIES}"
  "${SIONLIB_LIBRARIES}"
  "${BOOST_LIBRARIES}" )

if ( with-libraries )
  set( MODULE_LINK_LIBS "${MODULE_LINK_LIBS};${with-libraries}" )
endif ()
string( REPLACE ";" " " MODULE_LINK_LIBS "${MODULE_LINK_LIBS}" )

# libraries requied to link NEST
set( ALL_LIBS
  "-lnest"
  ${MODULE_LINK_LIBS} )


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
    "${PROJECT_SOURCE_DIR}/bin/nest-config.in"
    "${PROJECT_BINARY_DIR}/bin/nest-config" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/bin/nest_vars.sh.in"
    "${PROJECT_BINARY_DIR}/bin/nest_vars.sh" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/doc/fulldoc.conf.in"
    "${PROJECT_BINARY_DIR}/doc/fulldoc.conf" @ONLY
)

configure_file(
    "${PROJECT_SOURCE_DIR}/pynest/nest/versionchecker.py.in"
    "${PROJECT_BINARY_DIR}/pynest/nest/versionchecker.py" @ONLY
)


################################################################################
##################            Install Extra Files             ##################
################################################################################

install( FILES LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )

nest_print_config_summary()
