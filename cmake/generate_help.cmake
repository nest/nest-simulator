# generate_help.cmake
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

# needs the arguments:
# -DDOC_DIR=...
# -DDATA_DIR=...
# -DHELPDIRS=...
# -DINSTALL_DIR=...

if ( NOT CMAKE_CROSSCOMPILING )
  message( "Generate help from these directories:\n${HELPDIRS}" )

  # set environment vars
  set( ENV{NEST_DOC_DIR} "${DOC_DIR}" )
  set( ENV{NEST_DATA_DIR} "${DATA_DIR}" )
  set( ENV{NESTRCFILENAME} "/dev/null" )
  execute_process(
      COMMAND ${INSTALL_DIR}/bin/sli --userargs=${HELPDIRS}
                                              "${DATA_DIR}/sli/install-help.sli"
      RESULT_VARIABLE RET_VAR
      OUTPUT_FILE "install-help.log"
      ERROR_FILE "install-help.log"
  )
  # unset environment vars
  unset( ENV{NEST_DOC_DIR} )
  unset( ENV{NEST_DATA_DIR} )
  unset( ENV{NESTRCFILENAME )

  if ( RET_VAR EQUAL 0 )
    message( "SUCCESS" )
  else ()
    message( "ERROR: ${RET_VAR}" )
  endif ()

endif ()
