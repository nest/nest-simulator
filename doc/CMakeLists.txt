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

# Determine in or out of tree building
if ( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
  set( OUT_OF_TREE_BUILD "False" )
else ()
  set( OUT_OF_TREE_BUILD "True" )
endif ()

# Determine doc paths
set( DOC_BUILD_DIR "${PROJECT_BINARY_DIR}/doc/htmldoc" )
set( DOC_SOURCE_DIR "${PROJECT_SOURCE_DIR}/doc/htmldoc" )

if ( BUILD_SPHINX_DOCS )
  # Configure the Sphinx documentation.
  # 1. In out-of-tree builds, we need to copy over the source doc dir to the build dir,
  #    exact reasons currently unknown.
  # 2. The NEST source directory is passed to Sphinx by setting env:NESTSRCDIR, so that
  #    Sphinx can rewrite the source `pynestkernel.pyx` into a mocked `pynestkernel.py`
  # 3. Sphinx builds the documentation into the build directory, which then later gets
  #    installed into the target docdir.
  # 4. Sphinx generates the models RST documentation from the models/*.h files. But we
  #    need to copy them into a deeper directory.
  # 5. To make Python's `help` results more useful, `include` directives are inlined using
  #    `resolve_includes.py`
  # 6. This assembled Sphinx folder is marked to be installed, together with the logos.
  add_custom_target( sphinxdocs
  COMMAND [ "${OUT_OF_TREE_BUILD}" = "True" ] && rm -rf "${DOC_BUILD_DIR}" || true
  COMMAND [ "${OUT_OF_TREE_BUILD}" = "True" ] && cp -r "${DOC_SOURCE_DIR}" "${DOC_BUILD_DIR}" || true
    COMMAND NESTSRCDIR=${PROJECT_SOURCE_DIR} ${SPHINX_EXECUTABLE} -b html
    "${DOC_BUILD_DIR}"
    "${DOC_BUILD_DIR}/html"
  COMMAND cp "${DOC_BUILD_DIR}/models/*.rst" "${DOC_BUILD_DIR}/html/models"
    COMMAND ${Python_EXECUTABLE} "${DOC_BUILD_DIR}/resolve_includes.py" "${DOC_BUILD_DIR}/html/models"
)
  add_dependencies( docs sphinxdocs )

install( DIRECTORY "${DOC_BUILD_DIR}/html"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}"
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
