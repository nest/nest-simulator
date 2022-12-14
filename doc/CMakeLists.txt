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

if ( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
  set( OUT_OF_TREE_BUILD "False" )
else ()
  set( OUT_OF_TREE_BUILD "True" )
endif ()

set( DOC_BUILD_DIR "${PROJECT_BINARY_DIR}/doc/htmldoc" )
set( DOC_SOURCE_DIR "${PROJECT_SOURCE_DIR}/doc/htmldoc" )

add_custom_target( html
  COMMAND [ "${OUT_OF_TREE_BUILD}" = "True" ] && rm -rf "${DOC_BUILD_DIR}" || true
  COMMAND [ "${OUT_OF_TREE_BUILD}" = "True" ] && cp -r "${DOC_SOURCE_DIR}" "${DOC_BUILD_DIR}" || true
  COMMAND NESTSRCDIR=${PROJECT_SOURCE_DIR} sphinx-build -b html "${DOC_BUILD_DIR}" "${DOC_BUILD_DIR}/html"
  COMMAND cp "${DOC_BUILD_DIR}/models/*.rst" "${DOC_BUILD_DIR}/html/models"
  COMMAND ${PYTHON_EXECUTABLE} "${DOC_BUILD_DIR}/resolve_includes.py" "${DOC_BUILD_DIR}/html/models"
)

install( DIRECTORY "${DOC_BUILD_DIR}/html"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}"
  OPTIONAL
)

install( DIRECTORY logos
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )
