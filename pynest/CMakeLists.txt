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

  if ( CYTHON_FOUND )
    include( UseCython )
    set_source_files_properties(
        pynestkernel.pyx
        PROPERTIES CYTHON_IS_CXX TRUE )

    cython_add_module( pynestkernel pynestkernel.pyx )
  else ()
    # require the source pynest/pynestkernel.cxx precompiled
    set( pynestkernel_generated_file "NOTFOUND" )
    if ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/pynestkernel.cxx )
      set( pynestkernel_generated_file pynestkernel.cxx )
    elseif ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/pynestkernel.cpp )
      set( pynestkernel_generated_file pynestkernel.cpp )
    endif ()

    if ( pynestkernel_generated_file STREQUAL "NOTFOUND" )
      message( FATAL_ERROR "Need the source pynestkernel.cxx/cpp precompiled in '${CMAKE_CURRENT_SOURCE_DIR}', "
          "e.g. use `cythonize pynestkernel.pyx` in that directory." )
    endif ()
    python_add_module( pynestkernel ${pynestkernel_generated_file} pynestkernel.pxd )
    if ( APPLE )
      set_target_properties( pynestkernel PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
    else ()
      target_link_libraries( pynestkernel ${PYTHON_LIBRARIES} )
    endif ()
  endif ( CYTHON_FOUND )

  target_link_libraries( pynestkernel
      nest_lib nestutil nestkernel random sli_lib
      ${SLI_MODULES} ${EXTERNAL_MODULE_LIBRARIES}
      )

  target_include_directories( pynestkernel PRIVATE
      ${PROJECT_BINARY_DIR}/libnestutil
      ${PROJECT_SOURCE_DIR}/libnestutil
      ${PROJECT_SOURCE_DIR}/nest
      ${PROJECT_SOURCE_DIR}/nestkernel
      ${PROJECT_SOURCE_DIR}/librandom
      ${PROJECT_SOURCE_DIR}/sli
      ${SLI_MODULE_INCLUDE_DIRS}
      ${PYTHON_INCLUDE_DIRS}
      )

  target_compile_definitions( pynestkernel PRIVATE
      -D_IS_PYNEST
      )

  install( CODE "execute_process(
    COMMAND ${PYTHON} setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/build
                               install --prefix=${CMAKE_INSTALL_PREFIX}
                                       --install-lib=${PYEXECDIR}
                                       --install-scripts=${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}
                                       --install-data=${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA_DIR}
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")"
      )
  install( TARGETS pynestkernel DESTINATION ${PYEXECDIR}/nest/ )
  install( FILES ${PROJECT_BINARY_DIR}/pynest/do_tests.py
      DESTINATION ${INSTALL_DATA_DIR}/extras )

  install( DIRECTORY examples/
      DESTINATION ${INSTALL_DOC_DIR}/examples/pynest )

  find_program( NOSETESTS nosetests )
  if ( NOT NOSETESTS STREQUAL "NOSETESTS-NOTFOUND" )
    add_test( NAME PyNEST
        COMMAND ${NOSETESTS} -v --with-xunit
        --xunit-file=${PROJECT_BINARY_DIR}/reports/pynest_tests.xml
        ${PYEXECDIR}/nest/tests )
  endif ()
endif ( HAVE_PYTHON )
