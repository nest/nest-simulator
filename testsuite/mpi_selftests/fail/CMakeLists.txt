# testsuite/mpi_selftests/fail/CMakeLists.txt
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

if ( HAVE_MPI )
  # add mpi_selftests/fail files
  file( GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli )

  foreach ( script ${scripts} )
    add_test( NAME mpi_selftests/fail/${script}
        COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/nest_indirect
        ${CMAKE_INSTALL_FULL_DOCDIR}/mpi_selftests/fail/${script} # use the installed version
        )
    set_tests_properties( mpi_selftests/fail/${script}
        PROPERTIES WILL_FAIL TRUE )
  endforeach ()
endif ()
