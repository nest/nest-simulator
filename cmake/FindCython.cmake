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

# Modifications copyright (C) 2004 The NEST Initiative

# Using the Cython executable that lives next to the Python executable
# if it is a local installation.
if ( Python_FOUND )
  get_filename_component( _python_path ${Python_EXECUTABLE} PATH )
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
      ERROR_VARIABLE CYTHON_ERR_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if ( RESULT EQUAL 0 )
    if ( "${CYTHON_VAR_OUTPUT}" STREQUAL "" )
      # In cython v0.29.3 the version string is written to stderr and not to stdout, as one would expect.
      set( CYTHON_VAR_OUTPUT "${CYTHON_ERR_OUTPUT}" )
    endif()
    string( REGEX REPLACE ".* ([0-9]+\\.[0-9]+(\\.[0-9]+)?).*" "\\1"
                          CYTHON_VERSION "${CYTHON_VAR_OUTPUT}" )
  else ()
    message( FATAL_ERROR "Cython error: ${CYTHON_ERR_OUTPUT}\nat ${CYTHON_EXECUTABLE}")
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
