# BlueGeneQ_GCC.cmake
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

include( Platform/BlueGeneQ_Base )
# add path to ppc gcc
set( ENV{PATH} "/bgsys/drivers/ppcfloor/gnu-linux/bin:$ENV{PATH}" )

# set the compiler
set( CMAKE_C_COMPILER powerpc64-bgq-linux-gcc CACHE FILEPATH "override C compiler" )
set( CMAKE_CXX_COMPILER powerpc64-bgq-linux-g++ CACHE FILEPATH "override C++ compiler" )

if ( static-libraries )
  __bluegeneq_setup_static( GNU CXX )
  __bluegeneq_setup_static( GNU C )
else ()
  __bluegeneq_setup_dynamic( GNU CXX )
  __bluegeneq_setup_dynamic( GNU C )
endif ()
