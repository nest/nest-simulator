message("Hi from testsuite...")

configure_file(
  "${PROJECT_SOURCE_DIR}/testsuite/do_tests.sh.in"
  "${PROJECT_BINARY_DIR}/testsuite/do_tests.sh" @ONLY
)

set(TESTSUBDIRS
  selftests
  unittests
  regressiontests
  manualtests
  mpitests
  mpi_selftests)

install(DIRECTORY ${TESTSUBDIRS}
  DESTINATION ${INSTALL_DOC_DIR})

install(PROGRAMS ${PROJECT_BINARY_DIR}/testsuite/do_tests.sh
  DESTINATION ${INSTALL_DATA_DIR}/extras)
