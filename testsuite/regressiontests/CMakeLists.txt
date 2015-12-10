
# add regressiontests files
file(GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.sli)

foreach(script ${scripts})
  add_test(NAME regressiontests/${script}
    COMMAND ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}/nest
    ${CMAKE_INSTALL_PREFIX}/${INSTALL_DOC_DIR}/regressiontests/${script} # use the installed version
  )
endforeach()
