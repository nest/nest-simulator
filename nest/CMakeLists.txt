# nest/CMakeLists.txt
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

set( nest_sources
    neststartup.cpp neststartup.h
    sli_neuron.h sli_neuron.cpp
    )

add_executable( nest main.cpp ${nest_sources} )

add_library( nest_lib ${nest_sources} )

target_link_libraries( nest
    nestutil nestkernel random sli_lib ${SLI_MODULES} ${EXTERNAL_MODULE_LIBRARIES} )

target_link_libraries( nest_lib
    nestutil nestkernel random sli_lib ${SLI_MODULES} ${EXTERNAL_MODULE_LIBRARIES} )

target_include_directories( nest PRIVATE
    ${PROJECT_BINARY_DIR}/nest
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${SLI_MODULE_INCLUDE_DIRS}
    )

target_include_directories( nest_lib PRIVATE
    ${PROJECT_BINARY_DIR}/nest
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${SLI_MODULE_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
    )

if ( HAVE_PYTHON )
  target_compile_definitions( nest_lib PRIVATE
      -D_IS_PYNEST
      )
endif ( HAVE_PYTHON )

install( TARGETS nest nest_lib
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    )

install( FILES sli/models-init.sli
    DESTINATION ${INSTALL_DATA_DIR}/sli )
