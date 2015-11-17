message("Hi from doc...")

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/normaldoc.conf.in"
    "${CMAKE_CURRENT_BINARY_DIR}/normaldoc.conf" @ONLY)

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/fulldoc.conf.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fulldoc.conf" @ONLY)

  add_custom_target(prep-doxygen-conf)

  add_custom_target(doc
    DEPENDS prep-doxygen-conf
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/normaldoc.conf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

  add_custom_target(fulldoc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/fulldoc.conf
    DEPENDS prep-doxygen-conf
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif(DOXYGEN_FOUND)
