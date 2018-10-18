# examples/MyModule/cmake/FindSFMLAUDIO.cmake
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

# This CMake script is an extention to the CMake find_package() function 
# to check for sfml-audio, a submodule of SFML.
# Usage:
#   - make this script available to CMake
#   list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )
#   include( ProcessDependencies )
#   - as per CMake convention, the script name `Find<packagename>.cmake`
#     defines the package name referred to in find_package()
#   find_package( SFMLAUDIO )

set ( SFMLAUDIO_FOUND False )
find_path ( SFML_INCLUDE_DIR SFML/Audio.hpp PATH_SUFFIXES )

if ( SFML_INCLUDE_DIR )
  set ( SFMLAUDIO_FOUND True )
  message( "-- sfml-audio (Audio.hpp) found in ${SFML_INCLUDE_DIR}")
endif ()

