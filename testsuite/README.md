# `testsuite` folder

When writing tests, please adhere to the following guidelines:

* Place tests in the most suitable subdirectory below.
* Tests that should be run on 2, 3 or 4 MPI ranks directly as `mpirun -np n pytest ...``
  need to go into the corresponding subdirectory in `mpi_direct`.
* Tests that should be run with different numbers of MPI ranks and results compared
  across the different rank numbers, need to go into `mpi_indirect`.
* New regression tests should be called `test-issue-XXX.py`, where `XXX` is the
  number of the Github issue which the test covers.
* For more specific guidelines regarding tests in one of the different phases,
  see the README.md file in the corresponding directory.
