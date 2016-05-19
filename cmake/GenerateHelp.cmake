# cmake/GenerateHelp.cmake
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

function(NEST_GENERATE_HELP)
  # DIRS for help
  #
  # base dirs + dirs/sli

  set( __HELPDIRS libnestutil sli librandom lib nestkernel nest ${SLI_MODULES} )
  foreach ( child ${__HELPDIRS} )
    if ( IS_DIRECTORY "${PROJECT_SOURCE_DIR}/${child}" )
      set( HELPDIRS "${HELPDIRS}${PROJECT_SOURCE_DIR}/${child}:" )
      # Automatically include sli directory if it exists
      if ( IS_DIRECTORY "${PROJECT_SOURCE_DIR}/${child}/sli" )
        set( HELPDIRS "${HELPDIRS}${PROJECT_SOURCE_DIR}/${child}/sli:" )
      endif ()
    endif ()
  endforeach ()

  # testsuite dirs
  file( GLOB children ${PROJECT_SOURCE_DIR}/testsuite/*/ )
  foreach ( child ${children} )
    if ( IS_DIRECTORY ${child} )
      set( HELPDIRS "${HELPDIRS}${child}:" )
    endif ()
    if ( IS_DIRECTORY ${child}/pass )
      set( HELPDIRS "${HELPDIRS}${child}/pass:" )
    endif ()
    if ( IS_DIRECTORY ${child}/fail )
      set( HELPDIRS "${HELPDIRS}${child}/fail:" )
    endif ()
  endforeach ()

  if ( NOT CMAKE_CROSSCOMPILING )
    # install help (depends on sli and lib/sli/* installed)
    install( CODE
        "execute_process(
          COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_INSTALL_FULL_DOCDIR}/help/sli
          COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_INSTALL_FULL_DOCDIR}/help/cc
          COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_INSTALL_FULL_DOCDIR}/help/helpindex.html
          COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_INSTALL_FULL_DOCDIR}/help/helpindex.hlp

          COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_FULL_DOCDIR}/help
          COMMAND ${CMAKE_COMMAND}
              -DDOC_DIR='${CMAKE_INSTALL_FULL_DOCDIR}'
              -DDATA_DIR='${CMAKE_INSTALL_FULL_DATADIR}'
              -DHELPDIRS='${HELPDIRS}'
              -DINSTALL_DIR='${CMAKE_INSTALL_PREFIX}'
              -P ${PROJECT_SOURCE_DIR}/cmake/generate_help.cmake
            WORKING_DIRECTORY \"${PROJECT_BINARY_DIR}\"
          )"
        )
        install( CODE
            "execute_process(
                COMMAND ${CMAKE_COMMAND} -E remove_directory \"${CMAKE_INSTALL_FULL_DOCDIR}/help2/sli\"
                COMMAND ${CMAKE_COMMAND} -E remove_directory \"${CMAKE_INSTALL_FULL_DOCDIR}/help2/cc\"
                COMMAND ${CMAKE_COMMAND} -E remove \"${CMAKE_INSTALL_FULL_DOCDIR}/help2/helpindex.html\"
                COMMAND ${CMAKE_COMMAND} -E remove \"${CMAKE_INSTALL_FULL_DOCDIR}/help2/helpindex.hlp\"
                COMMAND ${CMAKE_COMMAND} -E remove_directory \"${CMAKE_INSTALL_FULL_DOCDIR}/help2\"

                COMMAND ${PYTHON} parse_help.py \"${CMAKE_INSTALL_FULL_DOCDIR}/help2\"
                WORKING_DIRECTORY \"${PROJECT_SOURCE_DIR}/extras/userdoc/generator\"
              )"
            )
  endif ()
endfunction()
