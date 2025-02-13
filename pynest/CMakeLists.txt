# pynest/CMakeLists.txt
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

if ( HAVE_PYTHON )

  # We use python3 here directly, as some of the CI jobs don't seem to have PYTHON
  # or Python_EXECUTABLE set properly.
  execute_process(
    COMMAND "python3" "${PROJECT_SOURCE_DIR}/pynest/generate_exception_header.py"
    "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    # Uncomment for debugging: ECHO_OUTPUT_VARIABLE ECHO_ERROR_VARIABLE COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )

  if ( CYTHON_FOUND )
    include( UseCython )
    set_source_files_properties( nestkernel_api.pyx PROPERTIES CYTHON_IS_CXX TRUE )
    cython_add_module( nestkernel_api nestkernel_api.pyx )
  else ()
    message( FATAL_ERROR "Building PyNEST requires Cython." )
  endif ()

  # TODO PYNEST NG: Add models, once the refactoring of the module system is done.
  # PYNEST-NG: somehow it works without models being in here, but why?
  target_link_libraries( nestkernel_api nestutil nestkernel )

  if ( APPLE )
    set_target_properties( nestkernel_api PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
  else ()
    set_target_properties( nestkernel_api PROPERTIES LINK_FLAGS "-Wl,--no-as-needed" )
    target_link_libraries( nestkernel_api ${Python_LIBRARIES} )
  endif ()

  target_include_directories( nestkernel_api PRIVATE
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/pynest
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/pynest
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/nestkernel/spatial
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/models
    ${Python_INCLUDE_DIRS}
  )

  target_compile_definitions( nestkernel_api PRIVATE
    -D_IS_PYNEST
  )

  install( DIRECTORY nest/ ${PROJECT_BINARY_DIR}/pynest/nest/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${PYEXECDIR}/nest
    PATTERN "versionchecker.py.in" EXCLUDE
  )

  install( DIRECTORY examples/
      DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/pynest
  )

  install( TARGETS nestkernel_api DESTINATION ${PYEXECDIR}/nest/ )
endif ()
