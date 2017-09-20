# `testsuite` folder 

When writing tests, please adhere to the following guidelines:

* Test scripts in `selftests` test basic functionality of the test 
  framework and must be written in SLI.
  
* `mpitests` have to be written in SLI in a particular style. Please look at
  existing tests for examples and the `unittest::distributed_*` test functions.
  
* Tests in `unittests` and `regressiontests` should generally be written in
  SLI. They must be written in SLI if they require a working SLI interpreter
  or NEST kernel. Tests should use the 
  `unittest::{assert|pass|fail||crash|failbutnocrash}_or_die` test functions.
  
* Tests in `unittests` and `regressiontests` may be written in Python, but those
  tests will not have access to a SLI interpreter or NEST kernel. Such tests
  can be used, e.g., for source code inspection. They are run after the SLI
  tests in the same directories.

* New regression tests should be called `issues-XXX.sli`, where `XXX` is the 
  number of the Github issue which the test covers. 

* Tests requiring PyNEST (NEST kernel accessible from Python) must be placed 
  in the `pynest/nest/tests` directory. They should use the Python `unittest` 
  framework.

* Tests that need models which depend on the GSL need to be protected against
  NEST compiled without GSL by placing the following at the beginning of
  the test script (after loading `unittest`):

      statusdict/have_gsl :: not { exit_test_gracefully } if

  For tests using PyNEST, define
  
      HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")

  and then apply the following decorator to all pertaining test classes

      @unittest.skipIf(not HAVE_GSL, 'GSL is not available')
  
* Tests that require certain features, such as threads, MPI or GSL, need to be
  skipped in the case where NEST was compiled without support for this feature.
  This can be done by placing one of the following commands at the beginning of
  the test script (after loading `unittest`):
  
      skip_if_no_mpi
      skip_if_have_mpi
      skip_if_not_threaded
      skip_if_without_gsl
      skip_if_without_music

  or skipping for an unspecified reason, with the following:

      /skipped exit_test_gracefully

  `test_threads.py` shows how to skip PyNEST tests that require threading. 

* For more specific guidelines regarding tests in one of the different phases,
  see the README.md file in the corresponding directory.
