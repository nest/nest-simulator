# topology/CMakeLists.txt
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

set( topo_sources
    topologymodule.h
    topologymodule.cpp
    topology_names.h
    topology_names.cpp
    connection_creator.h
    connection_creator_impl.h
    connection_creator.cpp
    position.h
    layer.h
    layer_impl.h
    layer.cpp
    free_layer.h
    grid_layer.h
    mask.h
    mask_impl.h
    mask.cpp
    grid_mask.h
    ntree.h
    ntree_impl.h
    vose.h
    vose.cpp
    topology.h topology.cpp
    )

add_library( topology ${topo_sources} )
target_link_libraries( topology nestutil sli_lib random nestkernel )

target_include_directories( topology PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    )

install( TARGETS topology
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

file( GLOB topo_unittests "testsuite/unittests/*" )
install( FILES ${topo_unittests}
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/topology/unittests
    )

file( GLOB topo_mpitests "testsuite/mpitests/*" )
install( FILES ${topo_mpitests}
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/topology/mpitests
    )

FILTER_HEADERS("${topo_sources}" install_headers )
install( FILES ${install_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
