# doc/CMakeLists.txt
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

if( BUILD_DOCS )
  # If we hit this, any part of the documentation was configured to be built.
  # The top-level 'docs' target will contain all sub-documentations such as `sphinxdocs`
  # and `doxygendocs`. Using `ALL` we make it run on `make install` as well.
  add_custom_target( docs ALL )
endif()

if( BUILD_SLI_DOCS STREQUAL ON AND NOT CMAKE_CROSSCOMPILING )
  # This is a legacy code block marked for imminent removal together with SLI. (11/2022)
  # It does not follow the restructured `docs`-`subdocs` dependencies, and
  # configures SLI doc building to occur on `install` using `execute_process` calls.
  message( STATUS "Configuring SLI documentation")
  install( DIRECTORY slihelp_generator
      DESTINATION ${CMAKE_INSTALL_DATADIR}
      )
  # Extract help from all source files in the source code, put
  # them in doc/help and generate a local help index in the
  # build directory containing links to the help files.
  install( CODE
    "execute_process(
       COMMAND ${Python_EXECUTABLE} -B generate_help.py \"${PROJECT_SOURCE_DIR}\" \"${PROJECT_BINARY_DIR}\"
       WORKING_DIRECTORY \"${PROJECT_SOURCE_DIR}/doc/slihelp_generator\"
       )"
    )

  # Copy the local doc/help directory to the global installation
  # directory for documentation.
  install( DIRECTORY "${PROJECT_BINARY_DIR}/doc/help"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}"
    OPTIONAL
    )

  # Update the global help index to include all help files in
  # the global installation directory for documentation.
  install( CODE
    "execute_process(
       COMMAND ${Python_EXECUTABLE} -B generate_helpindex.py \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}\"
       WORKING_DIRECTORY \"${PROJECT_SOURCE_DIR}/doc/slihelp_generator\"
       )"
    )
endif ()

# Determine in or out of tree building
if ( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
  set( OUT_OF_TREE_BUILD "False" )
else ()
  set( OUT_OF_TREE_BUILD "True" )
endif ()


if ( BUILD_SPHINX_DOCS )
  message( STATUS "Configuring Sphinx documentation" )
  set( _SPHINX_SOURCE_DIR "${PROJECT_SOURCE_DIR}/doc/htmldoc" )
  set( _SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build/html" )
  add_custom_target( sphinxdocs
    WORKING_DIRECTORY ${_SPHINX_SOURCE_DIR}
    COMMAND ${Python_EXECUTABLE} clean_source_dirs.py
    COMMAND ${SPHINX_EXECUTABLE} -b html . ${_SPHINX_BUILD_DIR}
    COMMAND ${Python_EXECUTABLE} resolve_includes.py ${SPHINX_BUILD_DIR}/models
  )
  add_dependencies( docs sphinxdocs )

  install( DIRECTORY ${_SPHINX_BUILD_DIR}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}
    OPTIONAL
    )
  install( DIRECTORY logos
      DESTINATION ${CMAKE_INSTALL_DOCDIR}
      )
endif ()

if ( BUILD_DOXYGEN_DOCS )
  add_custom_target( doxygendocs
      COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/fulldoc.conf"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      )
  add_dependencies( docs doxygendocs )
endif ()
