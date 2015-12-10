if(HAVE_MPI)
  # add mpi_selftests/fail files
  file(GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli)
  
  foreach(script ${scripts})
    add_test(NAME mpi_selftests/fail/${script}
      COMMAND ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}/nest_indirect
      ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/mpi_selftests/fail/${script} # use the installed version
    )
    set_tests_properties(mpi_selftests/fail/${script}
      PROPERTIES WILL_FAIL TRUE)
  endforeach()
endif(HAVE_MPI)
