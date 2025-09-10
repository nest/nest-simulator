# models/CMakeLists.txt
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

set(models_sources
    models.h ${PROJECT_BINARY_DIR}/models/models.cpp
    binary_neuron.h
    weight_recorder.h weight_recorder.cpp # Required by CommonSynapseProperties
    cm_compartmentcurrents.h cm_compartmentcurrents.cpp
    cm_tree.h cm_tree.cpp
    rate_neuron_ipn.h
    rate_neuron_opn.h
    rate_transformer_node.h
    weight_optimizer.h weight_optimizer.cpp
    ${MODELS_SOURCES_GENERATED}
)

add_library(models STATIC ${models_sources})
set_target_properties(models
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(models nestutil sli_lib nestkernel)

target_include_directories(models PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/models
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/models
    ${PROJECT_SOURCE_DIR}/nestkernel
)

FILTER_HEADERS("${models_sources}" install_headers)
install(FILES ${install_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
