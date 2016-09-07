# extras/ConnPlotter/CMakeLists.txt
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

if ( HAVE_PYTHON )
  install( CODE "execute_process(
  COMMAND ${PYTHON} setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/build
                             install --prefix=${CMAKE_INSTALL_PREFIX}
                                     --install-lib=${CMAKE_INSTALL_PREFIX}/${PYEXECDIR}
                                     --install-scripts=${CMAKE_INSTALL_FULL_BINDIR}
                                     --install-data=${CMAKE_INSTALL_FULL_DATADIR}
    WORKING_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}\")"
      )

  install( FILES examples/connplotter_tutorial.py
      DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/ConnPlotter
      )

  install( FILES doc/connplotter_tutorial.pdf
      DESTINATION ${CMAKE_INSTALL_DOCDIR}/ConnPlotter
      )

endif ()
