message("Hi from examples/sli...")

set(EXAMPLES_SLI_DATA
  nestrc.sli
  )

install(FILES ${EXAMPLES_SLI_DATA}
  DESTINATION ${INSTALL_DOC_DIR}/examples)
