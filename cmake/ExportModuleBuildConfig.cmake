# cmake/ExportModuleBuildConfig.cmake
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

# Generates both ways downstream extension-module authors can build against
# NEST -- bin/nest-config (shell script) and the NESTSimulator CMake
# CONFIG-mode package (NESTSimulatorConfig(Version).cmake, see
# cmake/NESTSimulatorConfig.cmake.in) -- in one place, since both describe
# the same dependency info for two different audiences:
#
#   nest-config --cflags --includes --libs
#
#   find_package(NESTSimulator REQUIRED)
#   target_link_libraries(mymodule PRIVATE NEST::nest)

function( NEST_EXPORT_MODULE_BUILD_CONFIG )
  # --- nest-config's data ---

  if ( NOT CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "None" )
    set( ALL_CFLAGS "${CMAKE_C_FLAGS}" )
    set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS}" )
  elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
    set( ALL_CFLAGS "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_DEBUG}" )
    set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}" )
  elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "Release" )
    set( ALL_CFLAGS "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_RELEASE}" )
    set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}" )
  elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" )
    set( ALL_CFLAGS "${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
    set( ALL_CXXFLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
  elseif ( "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel" )
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

  # GSL/Boost/MPI/OpenMP includes, libraries, and compile options come from
  # their own real-or-dummy imported targets (cmake/ProcessOptions.cmake) --
  # one source of truth, read here once and reused below for both
  # nest-config and the NESTSimulatorConfig.cmake template. Flattened
  # recursively: e.g. GSL::gsl's own INTERFACE_LINK_LIBRARIES is just the
  # target name "GSL::gslcblas", not a usable path -- nest_flatten_target_info
  # resolves that (and anything else like it) down to real library paths.
  nest_flatten_target_info( GSL::gsl _gsl_libs _gsl_includes _gsl_opts )
  nest_flatten_target_info( Boost::headers _boost_libs _boost_includes _boost_opts )
  nest_flatten_target_info( MPI::MPI_CXX _mpi_libs _mpi_includes _mpi_opts )
  nest_flatten_target_info( OpenMP::OpenMP_CXX _openmp_libs _openmp_includes _openmp_opts )

  # Libraries required to link extension modules
  set( MODULE_LINK_LIBS
    "${LTDL_LIBRARIES}"
    "${_gsl_libs}"
    "${LIBNEUROSIM_LIBRARIES}"
    "${MUSIC_LIBRARIES}"
    "${_mpi_libs}"
    "${_openmp_libs}"
    "${SIONLIB_LIBRARIES}" )
  if ( with-libraries )
    set( MODULE_LINK_LIBS "${MODULE_LINK_LIBS};${with-libraries}" )
  endif ()
  string( REPLACE ";" " " MODULE_LINK_LIBS "${MODULE_LINK_LIBS}" )

  # All includes
  #
  # We need to add all includes with -I, especially prevent that headers
  # from homebrew are included with -isystem, which some FindPackage might
  # do, see https://github.com/nest/nest-simulator/pull/3544#issuecomment-4284243626.
  set( ALL_INCLUDES_tmp
    "${CMAKE_INSTALL_FULL_INCLUDEDIR}/nest"
    "${LTDL_INCLUDE_DIRS}"
    "${_gsl_includes}"
    "${LIBNEUROSIM_INCLUDE_DIRS}"
    "${MUSIC_INCLUDE_DIRS}"
    "${_openmp_includes}"
    "${_mpi_includes}"
    "${SIONLIB_INCLUDE}"
    "${_boost_includes}"
    ${with-includes} )
  if ( ALL_INCLUDES_tmp )
    list( REMOVE_DUPLICATES ALL_INCLUDES_tmp )
  endif ()
  set( ALL_INCLUDES "" )
  foreach ( INC ${ALL_INCLUDES_tmp} )
    if ( INC )
      set( ALL_INCLUDES "${ALL_INCLUDES} -I${INC}" )
    endif ()
  endforeach ()

  configure_file(
    "${PROJECT_SOURCE_DIR}/bin/nest-config.in"
    "${PROJECT_BINARY_DIR}/bin/nest-config" @ONLY
  )

  # --- NEST::nest INTERFACE target + NESTSimulatorConfig.cmake ---
  #
  # Bundles the same flags/includes/third-party libs that nest-config
  # promises into one INTERFACE target. Does not export
  # nestkernel/models/nestutil themselves: those aren't installed, and
  # modules don't link against them at build time anyway (dlopen'd into the
  # running NEST process at runtime, resolved via -undefined dynamic_lookup,
  # set up below for downstream modules too).
  #
  # The real target is named "nest", not e.g. "nest_module_interface":
  # install(EXPORT ... NAMESPACE NEST::) prefixes the real target's name,
  # not the ALIAS below, so a differently-named real target would export as
  # NEST::<that name> instead of NEST::nest.
  add_library( nest INTERFACE )
  add_library( NEST::nest ALIAS nest )

  target_compile_features( nest INTERFACE cxx_std_20 )

  target_include_directories( nest INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/thirdparty>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libnestutil>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/libnestutil>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/nestkernel>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/nestkernel/spatial>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/models>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/nest>
  )

  target_link_libraries( nest INTERFACE
    GSL::gsl
    Boost::headers
    MPI::MPI_CXX
    OpenMP::OpenMP_CXX
    ${LTDL_LIBRARIES}
    ${LIBNEUROSIM_LIBRARIES}
    ${MUSIC_LIBRARIES}
    ${SIONLIB_LIBRARIES}
  )

  target_link_options( nest INTERFACE
    "$<$<PLATFORM_ID:Darwin>:LINKER:-undefined,dynamic_lookup>"
  )

  install( TARGETS nest EXPORT NESTSimulatorTargets )
  install( EXPORT NESTSimulatorTargets
    FILE NESTSimulatorTargets.cmake
    NAMESPACE NEST::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NESTSimulator
  )

  include( CMakePackageConfigHelpers )
  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/NESTSimulatorConfigVersion.cmake"
    VERSION ${NEST_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/NESTSimulatorConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/NESTSimulatorConfig.cmake" @ONLY
  )
  install( FILES
    "${PROJECT_BINARY_DIR}/NESTSimulatorConfig.cmake"
    "${PROJECT_BINARY_DIR}/NESTSimulatorConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NESTSimulator
  )
endfunction()
