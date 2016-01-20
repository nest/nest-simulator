# - Find ltdl header and library
#

# This module defines
#  LTDL_FOUND, if false, do not try to use LTDL.
#  LTDL_INCLUDE_DIRS, where to find ltdl.h.
#  LTDL_LIBRARIES, the libraries to link against to use libltdl.
#  LTDL_VERSION, the library version
#  LIBTOOL_EXECUTABLE, executable that provide generalized library-building support services.
#
# As a hint allows LTDL_ROOT_DIR

find_path(LTDL_INCLUDE_DIRS 
    NAMES ltdl.h
    HINTS ${LTDL_ROOT_DIR}/include
)
find_library(LTDL_LIBRARIES
    NAMES ltdl
    HINTS ${LTDL_ROOT_DIR}/lib
)
find_program(LIBTOOL_EXECUTABLE
  NAMES glibtool libtool
  HINTS ${LTDL_ROOT_DIR}/bin
)

if( NOT LIBTOOL_EXECUTABLE STREQUAL "LIBTOOL_EXECUTABLE-NOTFOUND" )
  execute_process(
    COMMAND ${LIBTOOL_EXECUTABLE} --version
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE LTDL_VAR_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(RESULT EQUAL 0)
    string( REGEX REPLACE ".* ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" LTDL_VERSION ${LTDL_VAR_OUTPUT} )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LTDL
  FOUND_VAR
    LTDL_FOUND
  REQUIRED_VARS
    LTDL_LIBRARIES
    LTDL_INCLUDE_DIRS
  VERSION_VAR
    LTDL_VERSION
)

mark_as_advanced(LTDL_ROOT_DIR LTDL_INCLUDE_DIRS LTDL_LIBRARIES LIBTOOL_EXECUTABLE)
