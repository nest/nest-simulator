# JURON_GCC.cmake
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
set( CMAKE_C_COMPILER gcc CACHE FILEPATH "override C compiler" )
set( CMAKE_CXX_COMPILER g++ CACHE FILEPATH "override C++ compiler" )

if ( static-libraries )
  __juron_setup_static( GNU CXX )
  __juron_setup_static( GNU C )
else ()
  __juron_setup_dynamic( GNU CXX )
  __juron_setup_dynamic( GNU C )
endif ()
