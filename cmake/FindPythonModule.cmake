# FindPythonModule.cmake
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

# https://cmake.org/pipermail/cmake/2011-January/041666.html
function(find_python_module module)
  string(TOUPPER ${module} module_upper)

  if(NOT PY_${module_upper})
    if(ARGC GREATER 1 AND ARGV1 STREQUAL "REQUIRED")
      set(${module}_FIND_REQUIRED TRUE)
    endif()

    # A module's location is usually a directory, but for binary modules
    # it's a .so file.
    execute_process(COMMAND "${Python_EXECUTABLE}" "-c"
      "import re, ${module}; print(re.compile('/__init__.py.*').sub('',${module}.__file__))"
      RESULT_VARIABLE _${module}_status
      OUTPUT_VARIABLE _${module}_location
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT _${module}_status)
      set(HAVE_${module_upper} ON CACHE INTERNAL "")
      set(PY_${module_upper} ${_${module}_location} CACHE STRING
	"Location of Python module ${module}")
    else()
      set(HAVE_${module_upper} OFF CACHE INTERNAL "")
    endif()

  endif(NOT PY_${module_upper})

  find_package_handle_standard_args(PY_${module} DEFAULT_MSG PY_${module_upper})
endfunction(find_python_module)
