# cmake/NESTSimulatorConfigExport.cmake
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

# Producer side of find_package(NESTSimulator)'s CONFIG-mode search: builds
# and installs NESTSimulatorConfig(Version).cmake (see
# cmake/NESTSimulatorConfig.cmake.in) so downstream CMake-based
# extension-module authors can do:
#
#   find_package(NESTSimulator REQUIRED)
#   target_link_libraries(mymodule PRIVATE NEST::nest)

function( NEST_INSTALL_CMAKE_PACKAGE )
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

  # OpenMP_CXX_FLAGS/MPI_CXX_COMPILE_FLAGS are space-separated strings (fine
  # for nest-config.in's shell substitution); INTERFACE_COMPILE_OPTIONS
  # needs a proper semicolon-separated list, or e.g. "-Xclang -fopenmp"
  # arrives at the compiler as one malformed token.
  separate_arguments( _nest_openmp_compile_options UNIX_COMMAND "${OpenMP_CXX_FLAGS}" )
  separate_arguments( _nest_mpi_compile_options UNIX_COMMAND "${MPI_CXX_COMPILE_FLAGS}" )

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
