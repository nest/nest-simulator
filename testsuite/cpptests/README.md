# C++ unit tests

This directory contains C++-level unit tests. The tests are run using the
Boost.Test framework, and are compiled into a separate executable,
`run_all_cpptests` by CMake if NEST is configured with Boost.

Tests are written in header files, preferably one file per test suite. Each
header file must include the Boost.Test header, create a test suite and at
least one test. Validation of predicate values is done with the
`BOOST_REQUIRE()` macro.

A small example of a test suite header can look like this:

```cpp
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( foo_suite )

BOOST_AUTO_TEST_CASE( foo_case )
{
  int i = 1;

  BOOST_REQUIRE( i > 0 );  // passes
  BOOST_REQUIRE( i == 2 ); // fails
}

BOOST_AUTO_TEST_SUITE_END()
```

The header then has to be included in `run_all.cpp` to be included in the
executable.

For further documentation on using the Boost.Test framework, see the [Boost.Test
documentation](https://www.boost.org/doc/libs/1_69_0/libs/test/doc/html/index.html).