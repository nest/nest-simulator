# FindLibNeurosim.cmake
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

# - Find libneurosim header and library
#
# This module defines
#  LIBNEUROSIM_FOUND, if false, do not try to use libneurosim.
#  LIBNEUROSIM_INCLUDE_DIRS, where to find neurosim/connection_generator.h.
#  LIBNEUROSIM_LIBRARIES, the libraries to link against to use libneurosim.
#  LIBNEUROSIM_VERSION, the library version
#
# As a hint allows LIBNEUROSIM_ROOT.

find_path( LIBNEUROSIM_INCLUDE_DIR
    NAMES neurosim/connection_generator.h
    HINTS ${LIBNEUROSIM_ROOT}/include
    )
find_library( NEUROSIM_LIBRARY
    NAMES neurosim
    HINTS ${LIBNEUROSIM_ROOT}/lib
    )

if ( ${PYTHON_VERSION} VERSION_GREATER "3" )
    find_library( PYNEUROSIM_LIBRARY
        NAMES py3neurosim
        HINTS ${LIBNEUROSIM_ROOT}/lib
    )
else ()
    find_library( PYNEUROSIM_LIBRARY
        NAMES pyneurosim
        HINTS ${LIBNEUROSIM_ROOT}/lib
    )
endif ()

if ( EXISTS "${LIBNEUROSIM_INCLUDE_DIR}/neurosim/version.h" )
  file( STRINGS "${LIBNEUROSIM_INCLUDE_DIR}/neurosim/version.h"
                version_h_contents REGEX "define LIBNEUROSIM_VERSION" )
  string( REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1"
                        LIBNEUROSIM_VERSION ${version_h_contents} )
endif ()

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( LibNeurosim
  FOUND_VAR
    LIBNEUROSIM_FOUND
  REQUIRED_VARS
    NEUROSIM_LIBRARY
    PYNEUROSIM_LIBRARY
    LIBNEUROSIM_INCLUDE_DIR
  VERSION_VAR
    LIBNEUROSIM_VERSION
    )

if ( LIBNEUROSIM_FOUND )
  set( LIBNEUROSIM_LIBRARIES "${NEUROSIM_LIBRARY}" "${PYNEUROSIM_LIBRARY}" )
  set( LIBNEUROSIM_INCLUDE_DIRS "${LIBNEUROSIM_INCLUDE_DIR}" )
endif ()

mark_as_advanced( LIBNEUROSIM_ROOT LIBNEUROSIM_INCLUDE_DIR NEUROSIM_LIBRARY PYNEUROSIM_LIBRARY )
