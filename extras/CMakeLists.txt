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

install( DIRECTORY logos
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )

install( DIRECTORY help_generator
    DESTINATION ${CMAKE_INSTALL_DATADIR}
    )

install( FILES EditorSupport/vim/syntax/sli.vim
    DESTINATION ${CMAKE_INSTALL_DATADIR}/extras/EditorSupport/vim/syntax
    )

install( FILES nestrc.sli
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
    )

install( PROGRAMS
    ${PROJECT_BINARY_DIR}/extras/nest-config
    nest_indirect
    nest_serial
    nest-server
    ${PROJECT_BINARY_DIR}/extras/nest_vars.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

add_subdirectory( ConnPlotter )
