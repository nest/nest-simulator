# precise/CMakeLists.txt
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

set( precise_sources
    slice_ring_buffer.cpp slice_ring_buffer.h
    iaf_psc_alpha_ps.cpp iaf_psc_alpha_ps.h
    iaf_psc_alpha_canon.cpp iaf_psc_alpha_canon.cpp
    iaf_psc_delta_ps.cpp iaf_psc_delta_ps.h
    iaf_psc_exp_ps.cpp iaf_psc_exp_ps.h
    iaf_psc_exp_ps_lossless.cpp iaf_psc_exp_ps_lossless.h
    poisson_generator_ps.cpp poisson_generator_ps.h
    parrot_neuron_ps.cpp parrot_neuron_ps.h
    precisemodule.cpp precisemodule.h
    )

add_library( precise ${precise_sources} )
target_link_libraries( precise
    nestutil sli_lib nestkernel
    )

target_include_directories( precise PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/librandom
    ${PROJECT_SOURCE_DIR}/sli
    ${PROJECT_SOURCE_DIR}/nestkernel
    )

install( TARGETS precise
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

FILTER_HEADERS("${precise_sources}" install_headers )
install( FILES ${install_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
