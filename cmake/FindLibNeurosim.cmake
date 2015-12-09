# - Find libneurosim header and library
#

# This module defines
#  LIBNEUROSIM_FOUND, if false, do not try to use libneurosim.
#  LIBNEUROSIM_INCLUDE_DIR, where to find neurosim/connection_generator.h.
#  LIBNEUROSIM_LIBRARIES, the libraries to link against to use libneurosim.
#  LIBNEUROSIM_VERSION, the library version
#
# As a hint allows LIBNEUROSIM_ROOT.

find_path(LIBNEUROSIM_INCLUDE_DIR
  NAMES neurosim/connection_generator.h
  HINTS ${LIBNEUROSIM_ROOT}/include
)
find_library(NEUROSIM_LIBRARY
  NAMES neurosim
  HINTS ${LIBNEUROSIM_ROOT}/lib
)

find_library(PYNEUROSIM_LIBRARY
  NAMES pyneurosim
  HINTS ${LIBNEUROSIM_ROOT}/lib
)

if(EXISTS "${LIBNEUROSIM_INCLUDE_DIR}/neurosim/version.h" )
  file( STRINGS "${LIBNEUROSIM_INCLUDE_DIR}/neurosim/version.h" version_h_contents REGEX "define LIBNEUROSIM_VERSION" )
  string( REGEX REPLACE ".*([0-9].[0-9].[0-9]).*" "\\1" LIBNEUROSIM_VERSION ${version_h_contents} )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibNeurosim
  FOUND_VAR
    LIBNEUROSIM_FOUND
  REQUIRED_VARS
    NEUROSIM_LIBRARY
    PYNEUROSIM_LIBRARY
    LIBNEUROSIM_INCLUDE_DIR
  VERSION_VAR
    LIBNEUROSIM_VERSION
  )

set(LIBNEUROSIM_LIBRARIES ${NEUROSIM_LIBRARY} ${PYNEUROSIM_LIBRARY})

mark_as_advanced(LIBNEUROSIM_ROOT LIBNEUROSIM_INCLUDE_DIR LIBNEUROSIM_LIBRARIES
  NEUROSIM_LIBRARY PYNEUROSIM_LIBRARY)
