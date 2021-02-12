# testsuite/cpptests/CMakeLists.txt
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

if ( HAVE_BOOST )
  include_directories( ${BOOST_INCLUDE_DIR} )

  add_executable( run_all_cpptests run_all.cpp )
  add_dependencies( run_all_cpptests sli nest )

  target_link_libraries( run_all_cpptests ${BOOST_LIBRARIES} nestkernel )

  target_include_directories( run_all_cpptests PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/thirdparty
    )

  install( TARGETS run_all_cpptests RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
  add_test( NAME cpptests COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/run_all_cpptests )
endif ()
