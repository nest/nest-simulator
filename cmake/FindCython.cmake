# Find the Cython compiler.
#
# This code sets the following variables:
#
#  CYTHON_EXECUTABLE
#
# See also UseCython.cmake

#=============================================================================
# Copyright 2011 Kitware, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=============================================================================

# FindCython.cmake
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

# Use the Cython executable that lives next to the Python executable
# if it is a local installation.
find_package( PythonInterp )
if ( PYTHONINTERP_FOUND )
  get_filename_component( _python_path ${PYTHON_EXECUTABLE} PATH )
  find_program( CYTHON_EXECUTABLE
      NAMES cython cython.bat cython3
      HINTS ${_python_path}
      )
else ()
  find_program( CYTHON_EXECUTABLE
      NAMES cython cython.bat cython3
      )
endif ()

if ( NOT CYTHON_EXECUTABLE STREQUAL "CYTHON_EXECUTABLE-NOTFOUND" )
  execute_process(
      COMMAND ${CYTHON_EXECUTABLE} --version
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE CYTHON_VAR_OUTPUT
      ERROR_VARIABLE CYTHON_VAR_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if ( RESULT EQUAL 0 )
    string( REGEX REPLACE ".* ([0-9]+\\.[0-9]+(\\.[0-9]+)?).*" "\\1"
                          CYTHON_VERSION "${CYTHON_VAR_OUTPUT}" )
  endif ()
endif ()

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Cython
  FOUND_VAR
    CYTHON_FOUND
  REQUIRED_VARS
    CYTHON_EXECUTABLE
  VERSION_VAR
    CYTHON_VERSION
    )

mark_as_advanced( CYTHON_EXECUTABLE )
