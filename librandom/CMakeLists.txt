# librandom/CMakeLists.txt
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

set( random_sources
    librandom_exceptions.h
    exp_randomdev.h
    knuthlfg.h knuthlfg.cpp
    mt19937.h mt19937.cpp
    random_datums.h
    random_numbers.h random_numbers.cpp
    randomgen.h randomgen.cpp
    binomial_randomdev.h binomial_randomdev.cpp
    exp_randomdev.cpp
    gamma_randomdev.h gamma_randomdev.cpp
    normal_randomdev.h normal_randomdev.cpp
    lognormal_randomdev.h lognormal_randomdev.cpp
    poisson_randomdev.h poisson_randomdev.cpp
    randomdev.h randomdev.cpp
    clipped_randomdev.h
    uniform_randomdev.h uniform_randomdev.cpp
    uniformint_randomdev.h uniformint_randomdev.cpp
    gslrandomgen.h gslrandomgen.cpp
    gsl_binomial_randomdev.h gsl_binomial_randomdev.cpp
    random.h random.cpp
    )

add_library( random ${random_sources} )
target_link_libraries( random sli_lib nestutil )

add_executable( randomtest randomtest.cpp )
target_link_libraries( randomtest random sli_lib nestutil )

target_include_directories( random PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/sli
    )

target_include_directories( randomtest PRIVATE
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/sli
    )

install( TARGETS random
    LIBRARY DESTINATION ${INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
    )
