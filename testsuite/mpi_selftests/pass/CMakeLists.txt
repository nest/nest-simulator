if(HAVE_MPI)
  # add mpi_selftests/pass files
  file(GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli)
  
  foreach(script ${scripts})
    add_test(NAME mpi_selftests/pass/${script}
      COMMAND ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}/nest_indirect
      ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/mpi_selftests/pass/${script} # use the installed version
    )
  endforeach()
endif(HAVE_MPI)
