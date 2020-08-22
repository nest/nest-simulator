# JURON_Clang.cmake
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
set( CMAKE_C_COMPILER clang CACHE FILEPATH "override C compiler" )
set( CMAKE_CXX_COMPILER clang++ CACHE FILEPATH "override C++ compiler" )

set( OpenMP_C_FLAGS "-fopenmp" CACHE STRING "Compier flag for OpenMP parallelization" FORCE )
if ( with-openmp )
  if ( with-offload )
    set(OpenMP_CXX_FLAGS "-fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=$ENV{CUDA}" CACHE STRING "Compiler flag for OpenMP offloading" FORCE )
    set(OpenMP_C_FLAGS "-fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=$ENV{CUDA}" CACHE STRING "Compiler flag for OpenMP offloading" FORCE )
  else()
    set(OpenMP_CXX_FLAGS "-fopenmp" CACHE STRING "Compiler flags for OpenMP parallelization" FORCE )
    set(OpenMP_C_FLAGS "-fopenmp" CACHE STRING "Compiler flags for OpenMP parallelization" FORCE )
  endif()
endif()

if ( static-libraries )
  __juron_setup_static( LLVM CXX )
  __juron_setup_static( LLVM C )
else ()
  __juron_setup_dynamic( LLVM CXX )
  __juron_setup_dynamic( LLVM C )
endif ()
