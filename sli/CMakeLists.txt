# sli/CMakeLists.txt
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

set( sli_sources
    allocator.h allocator.cpp
    aggregatedatum.h
    arraydatum.cc arraydatum.h
    booldatum.cc booldatum.h
    callbackdatum.h
    charcode.cc charcode.h
    datum.cc datum.h
    dict.cc dict.h
    dictdatum.cc dictdatum.h
    dictstack.cc dictstack.h
    dictutils.cc dictutils.h
    doubledatum.cc doubledatum.h
    fdstream.cc fdstream.h
    filesystem.cc filesystem.h
    functional.h
    functiondatum.cc functiondatum.h
    genericdatum.h
    get_mem.c
    integerdatum.cc integerdatum.h
    interpret.cc interpret.h
    iostreamdatum.cc iostreamdatum.h
    literaldatum.cc
    lockptrdatum.h lockptrdatum_impl.h
    sharedptrdatum.h
    name.cc name.h
    slinames.h slinames.cpp
    namedatum.cc namedatum.h
    numericdatum.h numericdatum_impl.h
    oosupport.cc oosupport.h
    parser.cc parser.h
    parserdatum.h
    processes.cc processes.h
    scanner.cc scanner.h
    sli_io.cc sli_io.h
    sliactions.cc sliactions.h
    sliarray.cc sliarray.h
    slibuiltins.cc slibuiltins.h
    slicontrol.cc slicontrol.h
    slidata.cc slidata.h
    slidict.cc slidict.h
    sliexceptions.cc sliexceptions.h
    slifunction.h
    sligraphics.cc sligraphics.h
    slimath.cc slimath.h
    slimodule.cc slimodule.h
    sliregexp.cc sliregexp.h
    slistack.cc slistack.h
    slistartup.cc slistartup.h
    slitype.cc slitype.h
    slitypecheck.cc slitypecheck.h
    specialfunctionsmodule.cc specialfunctionsmodule.h
    stringdatum.cc stringdatum.h
    symboldatum.cc symboldatum.h
    tarrayobj.cc tarrayobj.h
    token.cc token.h
    tokenarray.cc tokenarray.h
    tokenstack.cc tokenstack.h
    tokenutils.cc tokenutils.h
    triedatum.cc triedatum.h
    typearray.h
    typechk.cc typechk.h
    utils.cc utils.h
    )

add_library( sli_lib ${sli_sources} )
target_link_libraries( sli_lib nestutil OpenMP::OpenMP_CXX )

# Make a separate target for linking against readline, so that
# pynestkernel does not need to link against readline and make
# loading different readlines a problem for Python. See this
# pull request for more information:
# https://github.com/nest/nest-simulator/pull/323
add_library( sli_readline gnureadline.cc gnureadline.h )
target_link_libraries( sli_readline sli_lib nestutil ${READLINE_LIBRARIES} OpenMP::OpenMP_CXX )

# add the executable
add_executable( sli puresli.cc )
target_link_libraries( sli sli_lib sli_readline ${GSL_LIBRARIES} OpenMP::OpenMP_CXX )

target_include_directories( sli PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    )

target_include_directories( sli_lib PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    )
set_target_properties( sli_lib
    PROPERTIES
    OUTPUT_NAME sli
    )

target_include_directories( sli_readline PRIVATE
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/libnestutil
    ${PROJECT_BINARY_DIR}/libnestutil
    )

install( TARGETS sli_readline sli_lib sli
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/nest
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/nest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

FILTER_HEADERS("${sli_sources}" install_headers )
install( FILES ${install_headers} gnureadline.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nest)
