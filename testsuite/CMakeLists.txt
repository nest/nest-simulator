set(TESTSUBDIRS
  selftests
  unittests
  regressiontests
  manualtests
  mpitests
  mpi_selftests)

# add sli tests
add_subdirectory(selftests)
add_subdirectory(unittests)
add_subdirectory(regressiontests)
add_subdirectory(mpi_selftests/fail)
add_subdirectory(mpi_selftests/pass)
add_subdirectory(mpitests)

install(DIRECTORY ${TESTSUBDIRS}
  DESTINATION ${INSTALL_DOC_DIR})

install(PROGRAMS ${PROJECT_BINARY_DIR}/testsuite/do_tests.sh
  DESTINATION ${INSTALL_DATA_DIR}/extras)
