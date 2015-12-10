if(HAVE_MPI)
  # add mpitests files
  file(GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli)
  
  foreach(script ${scripts})
    add_test(NAME mpitests/${script}
      COMMAND ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}/nest_indirect
      ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/mpitests/${script} # use the installed version
    )
  endforeach()
endif(HAVE_MPI)
