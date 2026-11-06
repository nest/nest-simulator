# cmake/TargetPropertyHelpers.cmake
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

# Guards against get_target_property()'s own behavior for an unset
# property: it returns the literal string "<var>-NOTFOUND", which would
# otherwise silently end up as a bogus entry if put straight into a list
# (e.g. on the dummy GSL::gsl/MPI::MPI_CXX/etc. targets used when a
# dependency is disabled, cmake/ProcessOptions.cmake). Returns the property
# as a raw CMake list (semicolon-separated), or an empty string if unset or
# the target doesn't exist.
function( NEST_GET_TARGET_PROPERTY_LIST target property result_var )
  set( _val "" )
  if ( TARGET ${target} )
    get_target_property( _val ${target} ${property} )
    if ( NOT _val )
      set( _val "" )
    endif ()
  endif ()
  set( ${result_var} "${_val}" PARENT_SCOPE )
endfunction()

# Recursively walks target's INTERFACE_LINK_LIBRARIES graph -- entries there
# can themselves be other targets (e.g. GSL::gsl links GSL::gslcblas, not
# just a plain library path), so a single non-recursive property read isn't
# enough to get a flat, usable list. Collects IMPORTED_LOCATION (libraries),
# INTERFACE_INCLUDE_DIRECTORIES, and INTERFACE_COMPILE_OPTIONS from the
# target itself and everything it transitively links; results deduped.
function( NEST_FLATTEN_TARGET_INFO target libs_var includes_var opts_var )
  set( _libs "" )
  set( _includes "" )
  set( _opts "" )
  if ( TARGET ${target} )
    nest_get_target_property_list( ${target} IMPORTED_LOCATION _loc )
    if ( _loc )
      list( APPEND _libs "${_loc}" )
    endif ()
    nest_get_target_property_list( ${target} INTERFACE_INCLUDE_DIRECTORIES _inc )
    if ( _inc )
      list( APPEND _includes ${_inc} )
    endif ()
    nest_get_target_property_list( ${target} INTERFACE_COMPILE_OPTIONS _opt )
    if ( _opt )
      list( APPEND _opts ${_opt} )
    endif ()
    nest_get_target_property_list( ${target} INTERFACE_LINK_LIBRARIES _link_libs )
    foreach ( _lib ${_link_libs} )
      if ( TARGET ${_lib} )
        nest_flatten_target_info( ${_lib} _sub_libs _sub_includes _sub_opts )
        list( APPEND _libs ${_sub_libs} )
        list( APPEND _includes ${_sub_includes} )
        list( APPEND _opts ${_sub_opts} )
      elseif ( _lib )
        list( APPEND _libs "${_lib}" )
      endif ()
    endforeach ()
  endif ()
  if ( _libs )
    list( REMOVE_DUPLICATES _libs )
  endif ()
  if ( _includes )
    list( REMOVE_DUPLICATES _includes )
  endif ()
  if ( _opts )
    list( REMOVE_DUPLICATES _opts )
  endif ()
  set( ${libs_var} "${_libs}" PARENT_SCOPE )
  set( ${includes_var} "${_includes}" PARENT_SCOPE )
  set( ${opts_var} "${_opts}" PARENT_SCOPE )
endfunction()
