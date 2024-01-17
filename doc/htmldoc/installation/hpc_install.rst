.. _hpc_install:

High Performance Computer Systems Installation
================================================

How you want to install NEST on a supercomputer or cluster will depend on your needs
and how the system is set up.



Source install
---------------

See our full guide to installing :ref:`NEST from source <dev_install>`, which allows you to control
the configuration options.


Minimal configuration
~~~~~~~~~~~~~~~~~~~~~~


NEST can be compiled without any external packages; such a configuration may be useful for initial porting to a new supercomputer.
However, this implies several restrictions:

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


NEST docker
-----------

Alternatively, you can set up the pre-built :ref:`NEST docker container <docker>` that contains
everything you need for NEST, as well as options like NEST server, Jupyterlab, or :doc:`NEST Desktop <desktop:index>`.
Our docker container also comes with :doc:`NESTML <nestml:index>`.

Connect to remote machine - NEST Server
---------------------------------------

NEST Server can be deployed on a remote machine such as a workstation or cluster, and users
can access the NEST simulation engine through the RESTful API on their laptops.

See the :ref:`guide to NEST server <nest_server>` to learn more.


.. seealso::


   Check out our guides to :ref:`optimizing NEST for HPC systems <optimize_performance>`
