# - Find ltdl header and library
#

# This module defines
#  LTDL_FOUND, if false, do not try to use LTDL.
#  LTDL_INCLUDE_DIR, where to find ltdl.h.
#  LTDL_LIBRARIES, the libraries to link against to use libltdl.

find_path(LTDL_INCLUDE_DIR ltdl.h)
find_library(LTDL_LIBRARY NAMES ltdl)

if(LTDL_INCLUDE_DIR AND LTDL_LIBRARY)
    set(LTDL_FOUND 1)
    set(LTDL_LIBRARIES ${LTDL_LIBRARY})
    set(LTDL_INCLUDE_DIR ${LTDL_INCLUDE_DIR})
else(LTDL_INCLUDE_DIR AND LTDL_LIBRARY)
    set(LTDL_FOUND 0)
    set(LTDL_LIBRARIES)
    set(LTDL_INCLUDE_DIR)
endif(LTDL_INCLUDE_DIR AND LTDL_LIBRARY)

if(NOT LTDL_FOUND)
    set(LTDL_ERROR_MESSAGE  "LTDL was not found. Make sure LTDL_LIBRARY and LTDL_INCLUDE_DIR are set.")
    if(LTDL_FIND_REQUIRED)
        message(FATAL_ERROR ${LTDL_ERROR_MESSAGE})
    elseif(not LTDL_FIND_QUIETLY)
        message(STATUS ${LTDL_ERROR_MESSAGE})
    endif(LTDL_FIND_REQUIRED)
endif(NOT LTDL_FOUND)
