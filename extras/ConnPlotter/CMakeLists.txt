
if( HAVE_PYTHON )
install(CODE "execute_process(
  COMMAND ${PYTHON} setup.py build --build-base=${CMAKE_CURRENT_BINARY_DIR}/build
                             install --prefix=${CMAKE_INSTALL_PREFIX}
                                     --install-lib=${PYEXECDIR}
                                     --install-scripts=${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}
                                     --install-data=${CMAKE_INSTALL_PREFIX}/${INSTALL_DATA_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")

install(FILES examples/connplotter_tutorial.py
  DESTINATION ${INSTALL_DOC_DIR}/examples/ConnPlotter)

install(FILES doc/connplotter_tutorial.pdf
  DESTINATION ${INSTALL_DOC_DIR}/ConnPlotter)

endif( HAVE_PYTHON )
