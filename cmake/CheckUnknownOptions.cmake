# cmake/CheckUnknownOptions.cmake
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

# Catch typos in -D options. A cache variable that was passed on the command
# line but never declared via set(... CACHE ...) anywhere in the project
# stays at TYPE=UNINITIALIZED -- this is the same marker CMake's own
# end-of-run "Manually-specified variables were not used by the project"
# warning is based on. We turn it into a hard error here, restricted to
# names matching NEST's own option naming, so we never misfire on unrelated
# CMake-internal or generator-specific cache entries.
function( NEST_CHECK_UNKNOWN_OPTIONS )
  set( _nest_known_bare_options
    cythonize-pynest static-libraries tics_per_ms tics_per_step target-bits-split
  )
  get_cmake_property( _nest_cache_vars CACHE_VARIABLES )
  set( _nest_unknown_options "" )
  foreach( _v ${_nest_cache_vars} )
    if ( "${_v}" MATCHES "^with-" OR "${_v}" IN_LIST _nest_known_bare_options )
      get_property( _nest_var_type CACHE ${_v} PROPERTY TYPE )
      if ( _nest_var_type STREQUAL "UNINITIALIZED" )
        list( APPEND _nest_unknown_options "${_v}" )
      endif ()
    endif ()
  endforeach ()
  if ( _nest_unknown_options )
    printError( "Unknown option(s) given on the command line: ${_nest_unknown_options}. Please check for typos." )
  endif ()
endfunction()
