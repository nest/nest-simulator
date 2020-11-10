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


if ( NOT CMAKE_CROSSCOMPILING )

  # Python is needed to generate the help. If Python does not exist,
  # there are problems with the following.
  # See https://github.com/nest/nest-simulator/issues/678.
  find_package( PythonInterp )
  if ( PYTHONINTERP_FOUND )

    # Extract help from all source files in the source code, put
    # them in doc/help and generate a local help index in the
    # build directory containing links to the help files.
    install( CODE
      "execute_process(
         COMMAND ${PYTHON_EXECUTABLE} -B generate_help.py \"${PROJECT_SOURCE_DIR}\" \"${PROJECT_BINARY_DIR}\"
         WORKING_DIRECTORY \"${PROJECT_SOURCE_DIR}/extras/help_generator\"
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
         COMMAND ${PYTHON_EXECUTABLE} -B generate_helpindex.py \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}\"
         WORKING_DIRECTORY \"${PROJECT_SOURCE_DIR}/extras/help_generator\"
         )"
      )

  endif ()

endif ()

install( DIRECTORY model_details
    DESTINATION ${CMAKE_INSTALL_DOCDIR} )

install( FILES index.html doc_header.txt
    DESTINATION ${CMAKE_INSTALL_DOCDIR} )

if ( DOXYGEN_FOUND )
  add_custom_target( doc
      COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/normaldoc.conf"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
      )

  if ( DOXYGEN_DOT_FOUND )
    add_custom_target( fulldoc
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/fulldoc.conf"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
  else ()
    add_custom_target( fulldoc
        COMMAND ${CMAKE_COMMAND} -E echo "dot command is not found on your system. Cannot build full documentation."
        )
  endif ()
else ()
  add_custom_target( doc
      COMMAND ${CMAKE_COMMAND} -E echo "Doxygen is not found on your system. Cannot build documentation."
      )
  add_custom_target( fulldoc
      COMMAND ${CMAKE_COMMAND} -E echo "Doxygen is not found on your system. Cannot build full documentation."
      )
endif ()
