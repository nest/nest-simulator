# examples/MyModule/CMakeLists.txt
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

cmake_minimum_required( VERSION 2.8.12 )

# 1) Name your module here, i.e. add later with -Dexternal-modules=my:
set( SHORT_NAME my )
#    the complete module name is here:
set( MODULE_NAME ${SHORT_NAME}module )
# 2) Add all your sources here
set( MODULE_SOURCES
    mymodule.h mymodule.cpp
    pif_psc_alpha.cpp pif_psc_alpha.h
    drop_odd_spike_connection.h
    )
# 3) We require a header name like this:
set( MODULE_HEADER ${MODULE_NAME}.h )
# containing the class description of the class extending the SLIModule

# 4) Leave the rest as is. All files in `sli` will be installed.

project( ${MODULE_NAME} CXX )

# VERSION exported to libnestutil/config.h, extras/create_release.sh
set( MODULE_VERSION_MAJOR 1 )
set( MODULE_VERSION_MINOR 0 )
set( MODULE_VERSION "${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}" )

# on OS X
set( CMAKE_MACOSX_RPATH ON )

set( with-nest OFF CACHE STRING "Specify the `nest-config` executable." )

if ( NOT with-nest )
  # try find the program ourselves
  find_program( NEST_CONFIG
      NAMES nest-config
      )
  if ( NEST_CONFIG STREQUAL "NEST_CONFIG-NOTFOUND" )
    message( FATAL_ERROR "Cannot find the program `nest-config`. Specify via -Dwith-nest=... ." )
  endif ()
else ()
  set( NEST_CONFIG ${with-nest} )
endif ()

# gather the options
# PREFIX
execute_process(
    COMMAND ${NEST_CONFIG} --prefix
    RESULT_VARIABLE RES_VAR
    OUTPUT_VARIABLE NEST_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if ( NOT RES_VAR EQUAL 0 )
  message( FATAL_ERROR "Cannot run `${NEST_CONFIG}`. Please specify correct `nest-config` via -Dwith-nest=... " )
endif ()

if ( ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local" )
  # no prefix given?
  set( CMAKE_INSTALL_PREFIX "${NEST_PREFIX}" CACHE STRING "Install path prefix, prepended onto install directories." FORCE )
endif ()

# CFLAGS
execute_process(
    COMMAND ${NEST_CONFIG} --cflags
    RESULT_VARIABLE RES_VAR
    OUTPUT_VARIABLE NEST_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# static-libraries
execute_process(
    COMMAND ${NEST_CONFIG} --static-libraries
    RESULT_VARIABLE RES_VAR
    OUTPUT_VARIABLE NEST_STATIC_LIB
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if ( NEST_STATIC_LIB )
  set( BUILD_SHARED_LIBS OFF )
else ()
  set( BUILD_SHARED_LIBS ON )
endif ( NEST_STATIC_LIB )

# libs
execute_process(
    COMMAND ${NEST_CONFIG} --libs
    RESULT_VARIABLE RES_VAR
    OUTPUT_VARIABLE NEST_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# compiler
execute_process(
    COMMAND ${NEST_CONFIG} --compiler
    RESULT_VARIABLE RES_VAR
    OUTPUT_VARIABLE NEST_COMPILER
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if ( NOT CMAKE_CXX_COMPILER STREQUAL ${NEST_COMPILER} )
  message( FATAL_ERROR "NEST compiler and CMAKE compiler differ. Please specify with `-DCMAKE_CXX_COMPILER=${NEST_COMPILER}` ." )
endif ()

set( INSTALL_BIN_DIR bin )
set( INSTALL_INCLUDES_DIR include )
set( INSTALL_LIB_DIR lib/nest )
set( INSTALL_DATA_DIR share/nest )
set( INSTALL_DOC_DIR share/doc/nest )

set( PKGSRCDIR ${PROJECT_SOURCE_DIR} )
set( PKGBUILDDIR ${PROJECT_BINARY_DIR} )
set( PKGDATADIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA_DIR} )
set( PKGDOCDIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR} )


# CPack stuff
set( CPACK_GENERATOR TGZ )
set( CPACK_SOURCE_GENERATOR TGZ )

set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "NEST Module ${MODULE_NAME}" )
set( CPACK_PACKAGE_VENDOR "NEST Initiative (http://www.nest-initiative.org/)" )

set( CPACK_PACKAGE_VERSION_MAJOR ${MODULE_VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${MODULE_VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION ${MODULE_VERSION} )

set( CPACK_SOURCE_IGNORE_FILES
    "\\\\.gitignore"
    "\\\\.git/"
    "\\\\.travis\\\\.yml"
    # ignore autotools stuff
    # TODO cleanup, once autotools stuff is removed
    "/libltdl/"
    "/debian/"
    ".*\\\\.m4"
    ".*\\\\.ac"
    "autom4te\\\\.cache/"
    "bootstrap\\\\.sh"

    # if we have in source builds
    "/build/"
    "/_CPack_Packages/"
    "CMakeFiles/"
    "cmake_install\\\\.cmake"
    "Makefile.*"
    "CMakeCache\\\\.txt"
    "CPackConfig\\\\.cmake"
    "CPackSourceConfig\\\\.cmake"
    )
set( CPACK_SOURCE_PACKAGE_FILE_NAME ${MODULE_NAME} )

set( CPACK_PACKAGE_INSTALL_DIRECTORY "${MODULE_NAME} ${MODULE_VERSION}" )
include( CPack )

# add make dist target
add_custom_target( dist
    COMMAND ${CMAKE_MAKE_PROGRAM} package_source
    # not sure about this... seems, that it will be removed before dist...
    # DEPENDS doc
    COMMENT "Creating a source distribution from ${MODULE_NAME}..."
)


if ( BUILD_SHARED_LIBS )
  # build module 
  add_library( ${MODULE_NAME}_module MODULE ${MODULE_SOURCES} )
  set_target_properties( ${MODULE_NAME}_module
      PROPERTIES
      COMPILE_FLAGS "${NEST_CXXFLAGS} -DLTX_MODULE"
      LINK_FLAGS "${NEST_LIBS}"
      PREFIX ""
      OUTPUT_NAME ${MODULE_NAME} )
  install( TARGETS ${MODULE_NAME}_module
      DESTINATION ${INSTALL_LIB_DIR}
      )
endif ()

# build dynamic/static library for linking from NEST
add_library( ${MODULE_NAME}_lib ${MODULE_SOURCES} )
if ( BUILD_SHARED_LIBS )
  target_compile_definitions( ${MODULE_NAME}_lib PRIVATE -DLINKED_MODULE )
endif ( BUILD_SHARED_LIBS )
set_target_properties( ${MODULE_NAME}_lib
    PROPERTIES
    COMPILE_FLAGS "${NEST_CXXFLAGS}"
    LINK_FLAGS "${NEST_LIBS}"
    OUTPUT_NAME ${MODULE_NAME} )

install( TARGETS ${MODULE_NAME}_lib
    DESTINATION ${INSTALL_LIB_DIR}
    )

install( FILES ${MODULE_HEADER} DESTINATION ${INSTALL_INCLUDES_DIR} )
install( DIRECTORY sli DESTINATION ${INSTALL_DATA_DIR} )

# install help (depends on sli installed)
set( HELPDIRS "${PKGSRCDIR}:${PKGSRCDIR}/sli" )

install( CODE
    "execute_process(COMMAND ${CMAKE_COMMAND}
          -DDOC_DIR='${PKGDOCDIR}'
          -DDATA_DIR='${PKGDATADIR}'
          -DHELPDIRS='${HELPDIRS}'
          -DINSTALL_DIR='${CMAKE_INSTALL_PREFIX}'
          -P ${PKGDOCDIR}/generate_help.cmake
        WORKING_DIRECTORY \"${PROJECT_BINARY_DIR}\"
      )"
    )

message( "" )
message( "-------------------------------------------------------" )
message( "${MODULE_NAME} Configuration Summary" )
message( "-------------------------------------------------------" )
message( "" )
message( "C++ compiler         : ${CMAKE_CXX_COMPILER}" )
message( "Build static libs    : ${NEST_STATIC_LIB}" )
message( "C++ compiler flags   : ${CMAKE_CXX_FLAGS}" )
message( "NEST compiler flags  : ${NEST_CXXFLAGS}" )
message( "NEST libraries flags : ${NEST_LIBS}" )
message( "" )
message( "-------------------------------------------------------" )
message( "" )
message( "You can build and install ${MODULE_NAME} now, using" )
message( "  make" )
message( "  make install" )
message( "" )
message( "${MODULE_NAME} will be installed to: ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}" )
message( "" )
