# - Find MUSIC header and library
#

# This module defines
#  MUSIC_FOUND, if false, do not try to use MUSIC.
#  MUSIC_INCLUDE_DIRS, where to find music.hh.
#  MUSIC_LIBRARIES, the libraries to link against to use MUSIC.
#  MUSIC_EXECUTABLE, the music executable.
#  MUSIC_VERSION, the library version
#
# As a hint allows MUSIC_ROOT_DIR.

find_path(MUSIC_INCLUDE_DIRS
  NAMES music.hh
  HINTS ${MUSIC_ROOT_DIR}/include
)
find_library(MUSIC_LIBRARIES
  NAMES music
  HINTS ${MUSIC_ROOT_DIR}/lib
)
find_program(MUSIC_EXECUTABLE
  NAMES music
  HINTS ${MUSIC_ROOT_DIR}/bin
)


if( NOT MUSIC_EXECUTABLE STREQUAL "MUSIC_EXECUTABLE-NOTFOUND" )
  execute_process(
    COMMAND ${MUSIC_EXECUTABLE} --version
    RESULT_VARIABLE RESULT
    OUTPUT_VARIABLE MUSIC_VAR_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(RESULT EQUAL 0)
    string( REGEX REPLACE "^MUSIC ([0-9]\\.[0-9]\\.[0-9]).*" "\\1" MUSIC_VERSION ${MUSIC_VAR_OUTPUT} )
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Music
  FOUND_VAR
    MUSIC_FOUND
  REQUIRED_VARS
    MUSIC_LIBRARIES
    MUSIC_INCLUDE_DIRS
    MUSIC_EXECUTABLE
  VERSION_VAR
    MUSIC_VERSION
  )

mark_as_advanced(MUSIC_ROOT_DIR MUSIC_INCLUDE_DIRS MUSIC_LIBRARIES MUSIC_EXECUTABLE)
