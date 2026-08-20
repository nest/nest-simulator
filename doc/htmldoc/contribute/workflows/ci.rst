.. _cont_integration:

Continuous Integration
======================

`Continuous Integration <http://en.wikipedia.org/wiki/Continuous_integration>`_ (CI) is a software development practice where quality control is continuously applied to the product as opposed to the traditional procedure of applying quality control after completing all development (during the so called integration phase). In essence, it is a way of decreasing the risks associated with the integration by spreading required efforts over time, which helps to improve on the quality of software, and to reduce the time taken to deliver it.

Stringent quality control is particularly important in the context of NEST, a neuronal network simulator with the emphasis on correctness, reproducibility and performance. However, given the limited amount of the available resources, it is wasteful to transfer the responsibility to re-run the test suite for all target platforms on every single code base change to the shoulders of the developers.

In order to address this problem, a continuous integration (CI) service has been set up, which allows for regular testing of changes that are getting into the tree and timely reporting of identified problems. This way, issues will be discovered earlier and the amount of efforts to fix them significantly decreased.


Platforms tested
----------------

- MacOS on AMD64
- Ubuntu Linux on AMD64


CI stages
---------

The current CI implementation is defined in `.github/workflows/nestbuildmatrix.yml <https://github.com/nest/nest-simulator/blob/master/.github/workflows/nestbuildmatrix.yml>`_, which in turn calls several helper scripts in the `build_support <https://github.com/nest/nest-simulator/blob/master/build_support>`_ directory. As an end-user, it should not be necessary to understand or change these files; their functionality is described in detail below.

#. Static checks

   This first stages check the code without building or running it.

   - clang-format checks C++ code formatting (whitespace, etc.). Please note that a specific version of clang-format is used for consistency. The maximum line length is set to 120 characters.

   - cppcheck are used for more detailed semantic (but still static) code analysis.

   - mypy, pylint, black and flake8 are used to statically check Python code. A few errors are intentionally ignored, defined in `tox.ini <https://github.com/nest/nest-simulator/blob/master/tox.ini>`_. Again, the maximum line length is 120 characters.

   Errors that occurred in this stage are printed at the end of the log, including a list of affected files.

#. Build of NEST Simulator

   For details about the CI stages, please see the GitHub Actions workflow file https://github.com/nest/nest-simulator/blob/master/.github/workflows/nestbuildmatrix.yml.

#. Unit testing

   If static checks and the NEST Simulator build completed successfully, the actual unit testing suite is run with a variety of system configurations (for example, with or without MPI, OpenMP, MUSIC, and so on).

   Errors in this stage are printed in the "Build report" table at the end of each stage.
