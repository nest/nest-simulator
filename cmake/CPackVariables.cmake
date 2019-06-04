# cmake/CPackVariables.cmake
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

function( NEST_SET_CPACK_VARIABLES )
  # CPack stuff
  set( CPACK_GENERATOR TGZ )
  set( CPACK_SOURCE_GENERATOR TGZ )

  set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "nest::The Neural Simulation Tool" )
  set( CPACK_PACKAGE_VENDOR "NEST Initiative (https://www.nest-initiative.org/)" )
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

endfunction()
