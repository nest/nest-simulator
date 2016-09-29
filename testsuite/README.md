# `testsuite` folder 

When writing tests, please adhere to the following guidelines:

* The test scripts in `selftests` and `mpitests` have to be written in SLI,
  as they test for basic functionality, which has to be checked on the lowest
  level.

* The tests in `unittests` and `regressiontests` may be written in either
  Python or SLI. They are executed by the corresponding executable according
  to their filename extension. Tests that are written in Python are run after
  SLI tests.

* Tests that need models which depend on the GSL need to be protected by a check
  like this, which should be above all actual code in the test script:

        % This test should only run if we have GSL
        statusdict/have_gsl :: not {statusdict/exitcodes/success :: quit_i} if

* Tests that need threading, need to be protected by a check like this, which
  should be above all actual code in the test script:

        is_threaded not { exit_test_gracefully } if

* For more specific guidelines regarding tests in one of the different phases,
  see the README.md file in the corresponding directory.
