.. _hpc_install:

High Performance Computer Systems Installation
================================================

Minimal configuration
-------------------------

NEST can be compiled without any external packages; such a configuration may be useful for initial porting to a new supercomputer. However, this implies several restrictions:

- Some neuron and synapse models will not be available, as they depend on ODE solvers from the GNU Scientific Library.
- The Python extension will not be available
- Multi-threading and parallel computing facilities will be disabled.

To configure NEST for compilation without external packages, use the following  command::

    cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> \
          -Dwith-python=OFF \
          -Dwith-gsl=OFF \
          -Dwith-readline=OFF \
          -Dwith-ltdl=OFF \
          -Dwith-openmp=OFF \
          </path/to/nest/source>

See the :ref:`CMake Options <cmake_options>` to  further adjust settings for your system.
