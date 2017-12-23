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
    generic_factory.h
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
    topology_parameter.h
    topology_parameter.cpp
    selector.h
    selector.cpp
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

install( FILES sli/topology-interface.sli
    DESTINATION ${CMAKE_INSTALL_DATADIR}/sli )

install( FILES doc/Topology_UserManual.pdf
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/topology )

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

if ( HAVE_PYTHON )
  install( CODE "execute_process(
    COMMAND ${PYTHON} setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/build
                               install --prefix=${CMAKE_INSTALL_PREFIX}
                                       --install-lib=${CMAKE_INSTALL_PREFIX}/${PYEXECDIR}
                                       --install-scripts=${CMAKE_INSTALL_FULL_BINDIR}
                                       --install-data=${CMAKE_INSTALL_FULL_DATADIR}
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")"
      )

  file( GLOB topo_examples "examples/*" )
  install( FILES ${topo_examples}
      DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/topology
      )
endif ()
