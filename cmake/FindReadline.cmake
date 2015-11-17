# GNU Readline library finder

if(NOT READLINE_FOUND)

  set(READLINE_ROOT_DIR ${READLINE_ROOT_DIR} /usr)

  find_path(READLINE_INCLUDE_DIR readline/readline.h
    HINTS ${READLINE_ROOT_DIR}
    PATH_SUFFIXES include)

  find_library(READLINE_LIBRARY readline
    HINTS ${READLINE_ROOT_DIR}
    PATH_SUFFIXES lib)

  find_library(NCURSES_LIBRARY ncurses)   # readline depends on libncurses
  if(NOT ${NCURSES_LIBRARY} STREQUAL "NCURSES_LIBRARY-NOTFOUND")
    find_library(NCURSES_LIBRARY termcap)   # readline depends on libncurses
  endif()
  
  if(NOT ${NCURSES_LIBRARY} STREQUAL "NCURSES_LIBRARY-NOTFOUND")
    find_library(NCURSES_LIBRARY ncursesw)   # readline depends on libncurses
  endif()
  
  if(NOT ${NCURSES_LIBRARY} STREQUAL "NCURSES_LIBRARY-NOTFOUND")
    find_library(NCURSES_LIBRARY curses)   # readline depends on libncurses
  endif()

  mark_as_advanced(READLINE_INCLUDE_DIR READLINE_LIBRARY NCURSES_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Readline DEFAULT_MSG
    READLINE_LIBRARY NCURSES_LIBRARY READLINE_INCLUDE_DIR)

  set(READLINE_INCLUDE_DIRS ${READLINE_INCLUDE_DIR})
  set(READLINE_LIBRARIES ${READLINE_LIBRARY} ${NCURSES_LIBRARY})

endif(NOT READLINE_FOUND)
