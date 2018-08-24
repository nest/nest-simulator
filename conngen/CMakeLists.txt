# conngen/CMakeLists.txt
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
set( conngen_sources
    conngenmodule.h conngenmodule.cpp
    conngendatum.h conngendatum.cpp
    conngen.h conngen.cpp
    )

add_library( conngen ${conngen_sources} )

target_link_libraries( conngen nestutil sli_lib nestkernel ${LIBNEUROSIM_LIBRARIES} )

target_include_directories( conngen PRIVATE
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/librandom
    )

install( TARGETS conngen
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

install( DIRECTORY sli
    DESTINATION ${CMAKE_INSTALL_DATADIR} )

FILTER_HEADERS("${conngen_sources}" install_headers )
install( FILES ${install_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
