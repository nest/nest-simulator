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
#
# In addition, trap old option names that have been renamed or removed, so
# users get a clear error message rather than the option being silently ignored.
function( NEST_CHECK_UNKNOWN_OPTIONS )
  # Flat list of (old_name, action, target) triples.
  #   action = "renamed"  ->  message says "renamed; please use -D<target>"
  #   action = "removed"  ->  message says "removed; <target>" (explanation)
  # No entry may contain a semicolon: CMake treats ; as a list separator even
  # inside quoted strings when building a list with set().
  set( _nest_deprecated_options
    # old_name                    action      target / explanation
    "static-libraries"            "renamed"   "with-static-linking"
    "target-bits-split"           "renamed"   "with-target-bits-split"
    "with-intel-compiler-flags"   "renamed"   "with-intel-compiler-strict-math"
    "cythonize-pynest"            "renamed"   "with-prebuilt-pynest-cxx"
    "with-python"                 "removed"   "Python is always required"
    "tics_per_ms"                 "removed"   "Set tics_per_ms at runtime via nest.set"
    "tics_per_step"               "removed"   "Set tics_per_ms and resolution at runtime via nest.set"
  )

  get_cmake_property( _nest_cache_vars CACHE_VARIABLES )

  # Check deprecated names first so the user gets a targeted message.
  list( LENGTH _nest_deprecated_options _dep_len )
  set( _idx 0 )
  while ( _idx LESS _dep_len )
    list( GET _nest_deprecated_options ${_idx} _old_name )
    math( EXPR _idx "${_idx} + 1" )
    list( GET _nest_deprecated_options ${_idx} _action )
    math( EXPR _idx "${_idx} + 1" )
    list( GET _nest_deprecated_options ${_idx} _target )
    math( EXPR _idx "${_idx} + 1" )
    if ( "${_old_name}" IN_LIST _nest_cache_vars )
      get_property( _type CACHE ${_old_name} PROPERTY TYPE )
      if ( _type STREQUAL "UNINITIALIZED" )
        if ( _action STREQUAL "renamed" )
          message( FATAL_ERROR
            "Option -D${_old_name} has been renamed. Please use -D${_target} instead." )
        else ()
          message( FATAL_ERROR
            "Option -D${_old_name} has been removed. ${_target}." )
        endif ()
      endif ()
    endif ()
  endwhile ()

  # Generic check: any command-line variable still UNINITIALIZED at this point
  # was not recognised by the project.
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
