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
#
# In addition, trap old option names that have been renamed, so users get a
# clear error message rather than the option being silently ignored.
function( NEST_CHECK_UNKNOWN_OPTIONS )
  # Deprecated option names with their replacements, stored as a flat list of
  # (old, new) pairs. If a user passes one of these on the command line it stays
  # UNINITIALIZED (we never call set() on it), so the generic loop below would
  # already catch it; but an explicit check lets us print a targeted message.
  set( _nest_deprecated_options
    "static-libraries"          "with-static-linking"
    "cythonize-pynest"          "(removed; Cython is always required)"
    "with-python"               "(removed; Python is always required)"
    "tics_per_ms"               "with-tics-per-ms"
    "tics_per_step"             "with-tics-per-step"
    "target-bits-split"         "with-target-bits-split"
    "with-intel-compiler-flags" "with-intel-compiler-strict-math"
  )

  get_cmake_property( _nest_cache_vars CACHE_VARIABLES )

  # Check deprecated names first so the user gets a targeted message.
  list( LENGTH _nest_deprecated_options _dep_len )
  set( _idx 0 )
  while ( _idx LESS _dep_len )
    list( GET _nest_deprecated_options ${_idx} _old_name )
    math( EXPR _idx "${_idx} + 1" )
    list( GET _nest_deprecated_options ${_idx} _new_name )
    math( EXPR _idx "${_idx} + 1" )
    if ( "${_old_name}" IN_LIST _nest_cache_vars )
      get_property( _type CACHE ${_old_name} PROPERTY TYPE )
      if ( _type STREQUAL "UNINITIALIZED" )
        message( FATAL_ERROR
          "Option -D${_old_name} has been renamed. Please use -D${_new_name} instead." )
      endif ()
    endif ()
  endwhile ()

  # Generic check: any NEST-style option (with- prefix) that is still
  # UNINITIALIZED was not recognised by the project.
  set( _nest_unknown_options "" )
  foreach( _v ${_nest_cache_vars} )
    if ( "${_v}" MATCHES "^with-" )
      get_property( _type CACHE ${_v} PROPERTY TYPE )
      if ( _type STREQUAL "UNINITIALIZED" )
        list( APPEND _nest_unknown_options "${_v}" )
      endif ()
    endif ()
  endforeach ()
  if ( _nest_unknown_options )
    printError( "Unknown option(s) given on the command line: ${_nest_unknown_options}. Please check for typos." )
  endif ()
endfunction()
