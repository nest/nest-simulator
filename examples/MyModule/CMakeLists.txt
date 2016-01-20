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

set( WITH_NEST OFF CACHE STRING "Specify the `nest-config` executable." )

if ( NOT WITH_NEST )
  # try find the program ourselves
  find_program( NEST_CONFIG
      NAMES nest-config
      )
  if ( NEST_CONFIG STREQUAL "NEST_CONFIG-NOTFOUND" )
    message( FATAL_ERROR "Cannot find the program `nest-config`. Specify via -DWITH_NEST=... ." )
  endif ()
else ()
  set( NEST_CONFIG ${WITH_NEST} )
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
  message( FATAL_ERROR "Cannot run `${NEST_CONFIG}`. Please specify correct `nest-config` via -DWITH_NEST=... " )
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

if ( BUILD_SHARED_LIBS )
  # build module 
  add_library( ${MODULE_NAME}_module MODULE ${MODULE_SOURCES} )
  set_target_properties( ${MODULE_NAME}_module
      PROPERTIES
      COMPILE_FLAGS "${NEST_CXXFLAGS}"
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
