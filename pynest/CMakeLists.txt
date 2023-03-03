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

if(HAVE_PYTHON)
  if(CYTHON_FOUND)
    include(UseCython)
    set_source_files_properties(nestkernel_api.pyx PROPERTIES CYTHON_IS_CXX TRUE)
    cython_add_module(nestkernel_api nestkernel_api.pyx)
  else()
    message(FATAL_ERROR "Building PyNEST requires Cython.")
  endif()

  target_link_libraries(nestkernel_api
    nestutil nestkernel
    ${EXTERNAL_MODULE_LIBRARIES}
  )

  target_include_directories(nestkernel_api PRIVATE
    ${PROJECT_BINARY_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_SOURCE_DIR}/pynest
    ${PROJECT_SOURCE_DIR}/nestkernel
    ${PROJECT_SOURCE_DIR}/nestkernel/spatial
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${SLI_MODULE_INCLUDE_DIRS}
    ${Python_INCLUDE_DIRS}
  )

  target_compile_definitions(nestkernel_api PRIVATE
    -D_IS_PYNEST
  )

  install(DIRECTORY nest/ ${PROJECT_BINARY_DIR}/pynest/nest/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${PYEXECDIR}/nest
    PATTERN "versionchecker.py.in" EXCLUDE
  )
  install(TARGETS nestkernel_api DESTINATION ${PYEXECDIR}/nest/)
  install(CODE "
      execute_process(
          COMMAND ${PYTHON} ${CMAKE_CURRENT_BINARY_DIR}/setup.py install_egg_info
            --install-dir=${CMAKE_INSTALL_PREFIX}/${PYEXECDIR}
          WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/${PYEXECDIR}\"
      )
      # Use Python's standard library `pkg_resources` module to find
      # the install requirements of `nest` as specified in `setup.py`.
      # Then feed them into the stdin pipe of `pip install`.
      execute_process(
          COMMAND ${PYTHON} -c \"import pkg_resources as pkg; print('\\\\n'.join(str(r) for r in pkg.get_distribution('nest-simulator').requires()))\"
          COMMAND ${PYTHON} -m pip install -r /dev/stdin
      )
      "
  )

  install(DIRECTORY examples/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples/pynest
  )
endif()
