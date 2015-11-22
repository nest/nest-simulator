
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
