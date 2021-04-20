Continuous Integration
======================

`Continuous Integration <http://en.wikipedia.org/wiki/Continuous_integration>`_ (CI) is a software development practice where quality control is continuously applied to the product as opposed to the traditional procedure of applying quality control after completing all development (during the so called integration phase). In essence, it is a way of decreasing the risks associated with the integration by spreading required efforts over time, which helps to improve on the quality of software, and to reduce the time taken to deliver it.

Stringent quality control is particularly important in the context of NEST, a neuronal network simulator with the emphasis on correctness, reproducibility and performance. However, given the limited amount of the available resources, it is wasteful to transfer the responsibility to re-run the test suite for all target platforms on every single code base change to the shoulders of the developers.

In order to address this problem, a `Travis <http://travis-ci.org/>`_ Continuous Integration (CI) service has been set up, which allows for regular testing of changes that are getting into the tree and timely reporting of identified problems. This way, issues will be discovered earlier and the amount of efforts to fix them significantly decreased.


Platforms tested
----------------

- MacOS on AMD64
- Ubuntu Linux on AMD64


CI stages
---------

The current CI implementation is defined in `.travis.yml <https://github.com/nest/nest-simulator/blob/master/.travis.yml>`_, which in turn calls several helper scripts in the `extras <https://github.com/nest/nest-simulator/blob/master/extras>`_ directory. As an end-user, it should not be necessary to understand or change these files; their functionality is described in detail below.

#. Static checks

   This first stage checks the code without building or running it.

   - clang-format checks C++ code formatting (whitespace, etc.). Please note that a specific version of clang-format is used for consistency. The maximum line length is set to 120 characters.

   - Vera++ and cppcheck are used for more detailed semantic (but still static) code analysis.

   - pycodestyle is used to statically check Python code. A few errors are intentionally ignored, defined in the variable ``PEP8_IGNORES`` in `extras/static_code_analysis.sh <https://github.com/nest/nest-simulator/blob/master/extras/static_code_analysis.sh>`_. Again, the maximum line length is 120 characters.

   Errors that occurred in this stage are printed at the end of the log, including a list of affected files.

#. Build of NEST Simulator

   To ensure that changes in the code do not increase the number of compiler warnings generated during the build, warnings are counted and compared to a hardcoded number in the function ``makebuild_summary()`` in `extras/parse_build_log.py <https://github.com/nest/nest-simulator/blob/master/extras/parse_build_log.py>`_. The number of counted and expected warnings is printed in the "Build report" table printed at the end of the stage. For changes that legitimately increase the number of warnings, these values should be changed as part of the pull request.

   The CI builds cover both the gcc and clang compilers.

#. Unit testing

   If static checks and the NEST Simulator build completed successfully, the actual unit testing suite is run with a variety of system configurations (for example, with or without MPI, OpenMP, MUSIC, and so on).

   Errors in this stage are printed in the "Build report" table at the end of each stage.
