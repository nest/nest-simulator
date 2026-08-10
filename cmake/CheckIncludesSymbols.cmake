# cmake/CheckIncludeSymbols.cmake
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

# Here we check for all required include headers, types, symbols and functions.

# Check types exist
include( CheckTypeSize )

# given a list, filter all header files
function( FILTER_HEADERS in_list out_list )
    set( tmp_list ${in_list} )
    list( FILTER tmp_list INCLUDE REGEX "^.*\\.h(pp)?$" )
    set( ${out_list} ${tmp_list} PARENT_SCOPE )
endfunction ()
