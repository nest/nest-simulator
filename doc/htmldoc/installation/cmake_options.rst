.. _cmake_options:

CMake Options for NEST
======================

Before compiling and installing NEST, the source code  has to be
configured with ``cmake``. In the simplest case, the commands::

    cmake <nest_source_dir>
    make
    make install

will build NEST and install it to the site-packages of your Python
environment.

.. note::

  If you want to specify an alternative install location, use
  ``-DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir>``. It needs to be
  writable by the user running the install command.


Choice of compiler
------------------

We :ref:`systematically test <cont_integration>` NEST using the GNU
gcc and the Clang compiler suites.  Compilation with other up-to-date
compilers should also work, but we do not regularly test against those
compilers and can thus only provide limited support.

To select a specific compiler, please add the following flags to your
``cmake`` command line::

    -DCMAKE_C_COMPILER=<C-compiler> -DCMAKE_CXX_COMPILER=<C++-compiler>

Options for configuring NEST
----------------------------

NEST allows for several configuration options for custom builds:

.. _modelset_config:

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

Select built-in models
~~~~~~~~~~~~~~~~~~~~~~

By default, NEST will compile and register *all* neuron and synapse
models that are shipped in the source distribution. This is very
convenient for an explorative development of simulation scripts, but
leads to quite long compilation times and is often not necessary.

There are two ways to restrict the set of built-in models to tailor
NEST to your needs:

+---------------------------------------+------------------------------------------------------------------------------+
| ``-Dwith-modelset=<modelset>``        | Specify the modelset to include. Sample configurations are in the            |
|                                       | `modelsets <https://github.com/nest/nest-simulator/tree/master/modelsets>`_  |
|                                       | directory in the top-level of the source tree. A modelset is just a file     |
|                                       | listing one model header files (without the .h filename extension) to scan   |
|                                       | for models.                                                                  |
|                                       | This option is mutually exclusive with -Dwith-models. [default=full].        |
+---------------------------------------+------------------------------------------------------------------------------+
| ``-Dwith-models=[<modellist>|OFF]``   | Specify the models to include as a semicolon-separated list of model header  |
|                                       | files (without the .h filename extension) that are to be scanned for models. |
|                                       | This option is mutually exclusive with -Dwith-modelset. [default=OFF].       |
+---------------------------------------+------------------------------------------------------------------------------+

Use Python to build PyNEST
~~~~~~~~~~~~~~~~~~~~~~~~~~

+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dwith-python=[OFF|ON]``                    | Build PyNEST [default=ON].                                     |
+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dcythonize-pynest=[OFF|ON]``               | Use Cython to cythonize pynestkernel.pyx [default=ON]. If OFF, |
|                                               | PyNEST has to be build from a pre-cythonized pynestkernel.pyx. |
+-----------------------------------------------+----------------------------------------------------------------+

For more details, see the :ref:`Python binding <compile_with_python>` section below.

.. _performance_cmake:

Maximize performance, reduce energy consumption
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following options help to optimize NEST for maximal performance and thus reduced energy consumption.

+------------------------------------------+-----------------------------------------------+
| ``-Dwith-optimize="-O3 -march=native"``  | Activate most compiler options that do not    |
|                                          | affect compliance with IEEE754 numerics and   |
|                                          | optimize for CPU type used                    |
+------------------------------------------+-----------------------------------------------+
| ``-Dwith-defines=-DNDEBUG``              | Disable all ``assert()`` statements in NEST   |
+------------------------------------------+-----------------------------------------------+

.. note::

   * In our experience, gains from these optimizations are not very large. It can still be sensible to test them,
     especially if you are going to perform a large number of simulations.
   * Your particular use case may contain edge cases during NEST execution that our extensive test suite has
     not covered. Internal consistency tests in NEST in the form of ``assert()`` statements can help to detect such
     edge cases. Using the optimization options above removes these internal checks and thus increases the
     risk that NEST will produce incorrect results. Therefore, use these options *only after you have performed
     multiple simulations of your specific model with default optimization settings* (i.e., ``-O2``), which leaves the assertions
     in place.
   * Using ``-march=native`` requires that you build NEST on the same CPU architecture as you will use to run it.
   * For the technically minded: Even just using ``-O3`` removes some ``assert()`` statements from NEST since we
     have wrapped some of them in functions, which get eliminated due to interprocedural optimization.



Select parallelization scheme
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------------------------+----------------------------------------------------------------+
| ``-Dwith-mpi=[OFF|ON]``                     | Build with MPI parallelization [default=OFF].                  |
|                                             |                                                                |
+---------------------------------------------+----------------------------------------------------------------+
| ``-Dwith-openmp=[OFF|ON|<OpenMP-Flag>]``    | Build with OpenMP multi-threading [default=ON]. Optionally set |
|                                             | OMP compiler flags.                                            |
+---------------------------------------------+----------------------------------------------------------------+

See also the section on :ref:`building with MPI <compile-with-mpi>` below.


Build documentation
~~~~~~~~~~~~~~~~~~~

+------------------------------+-------------------------------------------------------------+
| ``-Dwith-devdoc=[OFF|ON]``   | Build the developer (doxygen) documentation [default=OFF]   |
|                              |                                                             |
+------------------------------+-------------------------------------------------------------+
| ``-Dwith-userdoc=[OFF|ON]``  | Build the user (Sphinx) documentation [default=OFF]         |
|                              |                                                             |
+------------------------------+-------------------------------------------------------------+

If either documentation build is toggled to `ON`, you can then run ``make docs`` if you only want to
build the docs.

See also the :ref:`documentation workflow <doc_workflow>` for user-facing and technical docs.


External libraries
~~~~~~~~~~~~~~~~~~

+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-libneurosim=[OFF|ON|</path/to/libneurosim>]``| Build with `libneurosim <https://github.com/INCF/libneurosim>`_ [default=OFF]. Optionally      |
|                                                       | give the directory where libneurosim is installed.                                             |
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-music=[OFF|ON|</path/to/music>]``            | Build with `MUSIC <https://github.com/INCF/MUSIC>`_ [default=OFF]. Optionally give the         |
|                                                       | directory where MUSIC is installed.                                                            |
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-sionlib=[OFF|ON|</path/to/sionlib>]``        | Build with                                                                                     |
|                                                       | `sionlib <https://www.fz-juelich.de/ias/jsc/EN/Expertise/Support/Software/SIONlib/_node.html>`_|
|                                                       | [default=OFF]. Optionally give the directory where sionlib is installed.                       |
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-boost=[OFF|ON|</path/to/boost>]``            | Build with Boost [default=ON]. To set a specific Boost installation, give the install path.    |
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-readline=[OFF|ON|</path/to/readline>]``      | Build with GNU Readline library [default=ON]. To set a specific library, give the install path.|
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-ltdl=[OFF|ON|</path/to/ltdl>]``              | Build with ltdl library [default=ON]. To set a specific ltdl, give the  install path. NEST uses|
|                                                       | ltdl for dynamic loading of external user modules.                                             |
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-gsl=[OFF|ON|</path/to/gsl>]``                | Build with the GSL library [default=ON]. To set a specific library, give the install path.     |
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+
| ``-Dwith-hdf5=[OFF|ON|</path/to/hdf5>]``              | Build with `HDF5 <https://hdfgroup.org/>`_ library [default=OFF]. To set a specific library,   |
|                                                       | give the install path. HDF5 is required for SONATA support, see :ref:`nest_sonata`.            |
+-------------------------------------------------------+------------------------------------------------------------------------------------------------+

NEST properties
~~~~~~~~~~~~~~~

+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dtics_per_ms=[number]``                    | Specify elementary unit of time [default=1000 tics per ms].    |
+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dtics_per_step=[number]``                  | Specify resolution [default=100 tics per step].                |
+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dwith-threaded-timers=[OFF|ON]``           | Build with one internal timer per thread [default=ON].         |
|                                               | Multi-threaded timers can affect the performance.              |
+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dwith-detailed-timers=[OFF|ON]``           | Build with detailed internal time measurements [default=OFF].  |
|                                               | Detailed timers can affect the performance.                    |
+----------------------------------------------------------------------------------------------------------------+
| ``-Dwith-mpi-sync-timer=[OFF|ON]``            | Build with mpi synchronization barrier and timer [default=OFF].|
|                                               | Can affect the performance.                                    |
+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dtarget-bits-split=['standard'|'hpc']``    | Split of the 64-bit target neuron identifier type              |
|                                               | [default='standard']. 'standard' is recommended for most users.|
|                                               | If running on more than 262144 MPI processes or more than 512  |
|                                               | threads, change to 'hpc'.                                      |
+-----------------------------------------------+----------------------------------------------------------------+
| ``-Dwith-full-logging=[OFF|ON]``              | Write debug output to file ``dump_<num_ranks>_<rank>.log``     |
|                                               | [default=OFF]. Developers should wrap debugging output in      |
|                                               | macro ``FULL_LOGGING_ONLY()`` and call kernel::manager<KernelManager>().write_dump()`  |
|                                               | from inside it. The macro can contain almost any valid code.   |
+-----------------------------------------------+----------------------------------------------------------------+

Generic build configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~

+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dstatic-libraries=[OFF|ON]``                      | Build static executable and libraries [default=OFF].             |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-optimize=[OFF|ON|<list;of;flags>]``         | Enable user defined optimizations                                |
|                                                      | [default=ON (uses '-O2')]. When OFF, no '-O' flag is passed to   |
|                                                      | the compiler. Explicit compiler flags can be given; separate     |
|                                                      | multiple flags by ';'."                                          |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-warning=[OFF|ON|<list;of;flags>]``          | Enable user defined warnings [default=ON (uses '-Wall')].        |
|                                                      | Separate  multiple flags by ';'.                                 |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-debug=[OFF|ON|<list;of;flags>]``            | Enable user defined debug flags [default=OFF]. When ON, '-g' is  |
|                                                      | used. Separate  multiple flags by ';'.                           |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-intel-compiler-flags=[OFF|<list;of;flags>]``| User defined flags for the Intel compiler                        |
|                                                      | [default='-fp-model strict']. Separate multiple flags by ';'.    |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-cpp-std=[<C++ standard>]``                  | C++ standard to use for compilation [default='c++17'].           |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-libraries=[OFF|<list;of;libraries>]``       | Link additional libraries [default=OFF]. Give full path. Separate|
|                                                      | multiple libraries by ';'.                                       |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-includes=[OFF|<list;of;includes>]``         | Add additional include paths [default=OFF]. Give full path       |
|                                                      | without '-I'. Separate multiple include paths by ';'.            |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-defines=[OFF|<list;of;defines>]``           | Additional defines, e.g. '-DXYZ=1' [default=OFF]. Separate       |
|                                                      | multiple defines by ';'.                                         |
+------------------------------------------------------+------------------------------------------------------------------+
| ``-Dwith-version-suffix=[string]``                   | Set a user defined version suffix [default=''].                  |
+------------------------------------------------------+------------------------------------------------------------------+


.. _compile-with-mpi:

Configuring NEST for Distributed Simulation with MPI
----------------------------------------------------

NEST supports distributed simulations using the Message Passing
Interface (MPI). Depending on your setup, you have to use one of the
following steps in order to add support for MPI:

  1. Try ``-Dwith-mpi=ON`` as argument for ``cmake``.

  2. If 1. does not work, or you want to use a non-standard MPI, try
     ``-Dwith-mpi=/path/to/my/mpi``. The `mpi` directory should
     contain the `include`, `lib` and `bin` subdirectories of the MPI
     installation.

  3. IfO 2. does not work, but you know the correct compiler wrapper
     for your installation, try adding the following to the invocation
     of ``cmake``::

         -DMPI_CXX_COMPILER=myC++_CompilerWrapper \
         -DMPI_C_COMPILER=myC_CompilerWrapper -Dwith-mpi=ON

When running large-scale parallel simulations and recording from many
neurons, writing to ASCII files might become prohibitively slow due to
the large number of resulting files. By installing the `SIONlib
library <http://www.fz-juelich.de/jsc/sionlib>`_ and supplying its
installation path to the ``-Dwith-sionlib=<path>`` option when calling
``cmake``, you can enable the :ref:`recording backend for binary files
<recording_backends>`, which solves this problem.

In order to run the distributed tests upon ``make installcheck``, NEST
needs to know how to execute the launcher of your MPI implementation.
CMake is usually able to detect the command line for this, but you can
customize it using the follwing configuration variables (common
defaults are shown below)::

    -DMPIEXEC=/usr/bin/mpirun
    -DMPIEXEC_NUMPROCS_FLAG=-np
    -DMPIEXEC_PREFLAGS=
    -DMPIEXEC_POSTFLAGS=

The final command line is composed in the following way::

    $MPIEXEC $MPIEXEC_NUMPROC_FLAG <np> $MPIEXEC_PREFLAGS <prog> $MPIEXEC_POSTFLAGS <args>

For details on setting specific flags for your MPI launcher command,
see the `CMake documentation
<https://cmake.org/cmake/help/latest/module/FindMPI.html>`_.

See the :ref:`parallel_computing` to learn how to execute threaded and
distributed simulations with NEST.

.. _compile_with_libneurosim:

Support for libneurosim
-----------------------

In order to allow NEST to create connections using external libraries,
it provides support for the Connection Generator Interface from
*libneurosim*. To request the use of libneurosim, you have to use the
follwing switch for the invocation of ``cmake``. It expects either
*ON* or *OFF*, or the directory where libneurosim is installed::

    -Dwith-libneurosim=[OFF|ON|</path/to/libneurosim>]

For details on how to use the Connection Generator Interface, see the
:ref:`guide on connection generation <connection_generator>`.

.. _compile_with_python:

Python Binding (PyNEST)
-----------------------

Note that since NEST 3.0, support for Python 2 has been dropped. Please use Python 3 instead.

``cmake`` usually autodetects your Python installation.
In some cases ``cmake`` might not be able to localize the Python interpreter
and its corresponding libraries correctly. To circumvent such a problem following
``cmake`` built-in variables can be set manually and passed to ``cmake``::

  PYTHON_EXECUTABLE ..... path to the Python interpreter
  PYTHON_LIBRARY ........ path to libpython
  PYTHON_INCLUDE_DIR .... two include ...
  PYTHON_INCLUDE_DIR2 ... directories

 e.g.: Please note ``-Dwith-python=ON`` is the default::
  cmake -DCMAKE_INSTALL_PREFIX=<nest_install_dir> \
        -DPYTHON_EXECUTABLE=/usr/bin/python3 \
        -DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.4m.so \
        -DPYTHON_INCLUDE_DIR=/usr/include/python3.4 \
        -DPYTHON_INCLUDE_DIR2=/usr/include/x86_64-linux-gnu/python3.4m \
        <nest_source_dir>



Compiler-specific options
-------------------------

NEST has reasonable default compiler options for the most common compilers.

Intel compiler
~~~~~~~~~~~~~~

To ensure that computations obey the IEEE754 standard for floating point
numerics, the ``-fp-model strict`` flag is used by default, but can be
overridden with ::

      -Dwith-intel-compiler-flags="<intel-flags>"

Portland compiler
~~~~~~~~~~~~~~~~~

Use the ``-Kieee`` flag to ensure that computations obey the IEEE754 standard for floating point numerics.
