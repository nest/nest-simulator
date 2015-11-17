message("Hi from examples...")

add_subdirectory(sli)
add_subdirectory(nest)

install(FILES run_examples.sh
  DESTINATION ${INSTALL_DOC_DIR})
