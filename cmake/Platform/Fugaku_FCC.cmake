# Fugaku_FCC.cmake
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

# the name of the target operating system
set( CMAKE_SYSTEM_NAME Linux CACHE STRING "Cross-compiling for Fugaku powered by the A64FX CPU" )
set( CMAKE_SYSTEM_PROCESSOR "a64fx" )
set( TRIPLET_VENDOR fujitsu )

#
# Set Fugaku for main CMakeList.txt
#
set( Fugaku ON CACHE BOOL "Enable Fugaku." FORCE )
# no readline support on Fugaku
set( with-readline OFF CACHE BOOL "Find a readline library [default=ON]. To set a specific readline, set install path." FORCE )
set( with-ltdl OFF CACHE BOOL "Find a ltdl library [default=ON]. To set a specific ltdl, set install path." FORCE )
# we obviously want to do mpi on Fugaku
set( with-mpi ON CACHE BOOL "Request compilation with MPI; optionally give directory with MPI installation." FORCE )
set( static-libraries ON CACHE BOOL "Build static libraries." FORCE )

#
# Library prefixes, suffixes, extra libs.
#
set( CMAKE_LINK_LIBRARY_SUFFIX "" )
set( CMAKE_STATIC_LIBRARY_PREFIX "lib" )     # lib
set( CMAKE_STATIC_LIBRARY_SUFFIX ".a" )      # .a
set( CMAKE_EXECUTABLE_SUFFIX "" )            # .exe

#
# Library search prefix/suffix.
#
set( CMAKE_FIND_LIBRARY_PREFIXES "lib" )
set( CMAKE_FIND_LIBRARY_SUFFIXES ".a" )

# set the compiler
set( CMAKE_C_COMPILER fccpx CACHE FILEPATH "Override C compiler" )
set( CMAKE_CXX_COMPILER FCCpx CACHE FILEPATH "Override C++ compiler" )

# Prevent CMake from adding GNU-specific linker flags (-rdynamic)
set( CMAKE_C_COMPILER_ID "Fujitsu" CACHE STRING "Fujitsu C cross-compiler" FORCE )
set( CMAKE_CXX_COMPILER_ID "Fujitsu" CACHE STRING "Fujitsu C++ cross-compiler" FORCE )
