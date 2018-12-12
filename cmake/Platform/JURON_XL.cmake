# JURON_XL.cmake
#
# This file is part of NEST.
#
# Copyright (C) 2018 The NEST Initiative
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

include( Platform/JURON_Base )
# set the compiler
set( CMAKE_C_COMPILER xlC CACHE FILEPATH "override C compiler" )
set( CMAKE_CXX_COMPILER xlc++ CACHE FILEPATH "override C++ compiler" )

#
# Compile flags for different build types
#
set( CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "Compiler optimization flags" )
set( CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Compiler optimization flags" )
set( CMAKE_C_FLAGS_RELEASE "-O3 -qstrict -qarch=pwr8 -qtune=pwr8 -DNDEBUG" CACHE STRING "Compiler optimization flags" )
set( CMAKE_CXX_FLAGS_RELEASE "-O3 -qstrict -qarch=pwr8 -qtune=pwr8 -DNDEBUG" CACHE STRING "Compiler optimization flags" )
set( CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -qarch=pwr8 -qtune=pwr8 -DNDEBUG" CACHE STRING "Compiler optimization flags" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -qarch=pwr8 -qtune=pwr8 -DNDEBUG" CACHE STRING "Compiler optimization flags" )

set( OpenMP_C_FLAGS "-qsmp=omp -qoffload" CACHE STRING "Compiler flag for OpenMP parallelization" FORCE )
set( OpenMP_CXX_FLAGS "-qsmp=omp -qoffload" CACHE STRING "Compiler flag for OpenMP parallelization" FORCE )

set( with-warning "-Wall" CACHE STRING "Enable user defined warnings. [default ON, when ON, defaults to '-Wall']" )

if ( static-libraries )
  __juron_setup_static( XL CXX )
  __juron_setup_static( XL C )
else ()
  __juron_setup_dynamic( XL CXX )
  __juron_setup_dynamic( XL C )
endif ()
