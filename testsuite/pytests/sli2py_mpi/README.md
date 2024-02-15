# MPI tests

Test in this directory run NEST with different numbers of MPI ranks and compare results.

- The process is managed by subclasses of the `MPITestWrapper` base class
- Each test file must contain exactly one test function
- The test function must be decorated with a subclass of `MPITestWrapper`
- Test files **must not import nest** outside the test function
- `conftest.py` must not be loaded, otherwise mpirun will return a non-zero exit code; use `pytest --noconftest`
- The wrapper will write a modified version of the test file to a temporary directory and mpirun it from there; results are collected in the temporary directory
- Evaluation criteria are determined by the MPIWrapper subclass

This is still work in progress.
