# extras/CMakeLists.txt
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

execute_process( COMMAND ${PROJECT_SOURCE_DIR}/extras/create_rcsinfo.sh ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR} )
if ( EXISTS ${PROJECT_BINARY_DIR}/lib/sli/rcsinfo.sli )
  install( FILES ${PROJECT_BINARY_DIR}/lib/sli/rcsinfo.sli
      DESTINATION ${INSTALL_DATA_DIR}/sli )
endif ()

install( DIRECTORY bibliography logos
    DESTINATION ${INSTALL_DOC_DIR} )

install( FILES emacs/postscript-sli.el emacs/psvn.el ${CMAKE_CURRENT_BINARY_DIR}/emacs/sli.el
    DESTINATION ${INSTALL_DATA_DIR}/extras/emacs )

install( PROGRAMS
    ${PROJECT_BINARY_DIR}/extras/nest-config
    nest_indirect
    nest_serial
    ${PROJECT_BINARY_DIR}/extras/nest_vars.sh
    DESTINATION ${INSTALL_BIN_DIR} )

add_subdirectory( ConnPlotter )
