# Python-based MPI tests

This directory contains Python tests requiring MPI.

The name of each subdirectory is the number of MPI processes to be used.
Tests are to be run as

    mpirun -np N pytest <directory>

Tests can either be placed in the directories directly or as symlinks
to tests that shall also be run as serial tests. In this case, any
auxiliary files must also be linked.
