
# add unittests files
file(GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli)

foreach(script ${scripts})
  add_test(NAME unittests/${script}
    COMMAND ${CMAKE_INSTALL_PREFIX}/bin/nest
    ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/unittests/${script} # use the installed version
  )
endforeach()
