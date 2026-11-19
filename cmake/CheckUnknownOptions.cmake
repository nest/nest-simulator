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
# warning is based on. We turn it into a hard error here for ALL variables,
# because warnings are easily overlooked. All legitimate CMake, find_package(),
# and NEST variables are declared (and therefore no longer UNINITIALIZED) by
# the time this function runs, so anything still UNINITIALIZED is a typo or
# an obsolete option.
function( NEST_CHECK_UNKNOWN_OPTIONS )

  # Generic check: any command-line variable still UNINITIALIZED at this point was not recognised by the project.
  get_cmake_property( _nest_cache_vars CACHE_VARIABLES )

  set( _nest_unknown_options "" )
  foreach( _v ${_nest_cache_vars} )
    get_property( _type CACHE ${_v} PROPERTY TYPE )
    if ( _type STREQUAL "UNINITIALIZED" )
      list( APPEND _nest_unknown_options "${_v}" )
    endif ()
  endforeach ()
  if ( _nest_unknown_options )
    printError( "Unknown option(s) given on the command line: ${_nest_unknown_options}. Please check for typos." )
  endif ()

endfunction()
