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
check_type_size( "long long" LONG_LONG_SIZE LANGUAGE CXX ) # also sets HAVE_LONG_LONG
if ( LONG_LONG_SIZE GREATER 0 )
  set( HAVE_LONG_LONG ON )
endif ()

set( CMAKE_EXTRA_INCLUDE_FILES "stdint.h" )
check_type_size( uint64_t UINT64_T_SIZE LANGUAGE CXX )
set( CMAKE_EXTRA_INCLUDE_FILES OFF )
if ( UINT64_T_SIZE GREATER 0 )
    set( HAVE_UINT64_T ON )
endif ()

if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    set( CMAKE_EXTRA_INCLUDE_FILES "stdint.h" )
    check_type_size( "long" LONG_SIZE LANGUAGE CXX )
    if( NOT LONG_SIZE EQUAL 4 )
        printError( "Platform not supported: None-ILP32 data model for 32 bit architecture." )
    else ()
        set( HAVE_32BIT_ARCH ON )
    endif()
endif()

# Check symbols / defines exist
include( CheckCXXSymbolExists )
check_cxx_symbol_exists( NAN "cmath" HAVE_NAN )
check_cxx_symbol_exists( isnan "cmath" HAVE_ISNAN )
check_cxx_symbol_exists( M_E "cmath" HAVE_M_E )
check_cxx_symbol_exists( M_PI "cmath" HAVE_M_PI )
check_cxx_symbol_exists( expm1 "cmath" HAVE_EXPM1 )

# given a list, filter all header files
function( FILTER_HEADERS in_list out_list )
    if( ${CMAKE_VERSION} VERSION_LESS "3.6" )
        unset( tmp_list )
        foreach( fname ${in_list} )
            if( "${fname}" MATCHES "^.*\\.h(pp)?$" )
                list( APPEND tmp_list "${fname}" )
            endif ()
        endforeach ()
    else ()
        set( tmp_list ${in_list} )
        list( FILTER tmp_list INCLUDE REGEX "^.*\\.h(pp)?$")
    endif ()
    set( ${out_list} ${tmp_list} PARENT_SCOPE )
endfunction ()
