# cmake/FindSphinx.cmake
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

# Based on information found on https://www.vortech.nl/en/integrating-sphinx-in-cmake

include(FindPackageHandleStandardArgs)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
if( Python_FOUND )
    get_filename_component(_PYTHON_DIR "${PYTHON_EXECUTABLE}" DIRECTORY)
    set(
        _PYTHON_PATHS
        "${_PYTHON_DIR}"
        "${_PYTHON_DIR}/bin"
        "${_PYTHON_DIR}/Scripts"
        )
endif()

find_program(
    SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build.exe
    HINTS ${_PYTHON_PATHS}
    )
mark_as_advanced(SPHINX_EXECUTABLE)

find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)