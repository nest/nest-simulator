# Random123 tests

This directory contains known-answer-tests for the Random123 generators, with their own testing harness. As per the documentation for the Random123 library, the generators should not be used if any known-answer-test fails.

The tests are compiled and run during CMake configuration, and use test data from kat_vectors. If any of the tests fail, the generators are disabled.
