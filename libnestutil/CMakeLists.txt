# libnestutil/CMakeLists.txt
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

set( nestutil_sources
    beta_normalization_factor.h
    block_vector.h
    dict_util.h
    enum_bitfield.h
    iaf_propagator.h iaf_propagator.cpp
    iterator_pair.h
    lockptr.h
    logging_event.h logging_event.cpp
    logging.h
    nest_types.h
    numerics.h numerics.cpp
    regula_falsi.h
    sort.h
    string_utils.h
    vector_util.h
    )

add_library( nestutil STATIC ${nestutil_sources} )

set_target_properties( nestutil
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )

target_link_libraries( nestutil ${GSL_LIBRARIES} ${SIONLIB_LIBS} )

target_include_directories( nestutil PRIVATE
    ${PROJECT_BINARY_DIR}/libnestutil
    ${Boost_INCLUDE_DIRS}
    )

FILTER_HEADERS("${nestutil_sources}" install_headers )
install( FILES ${install_headers} ${PROJECT_BINARY_DIR}/libnestutil/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
