Configuration Options
=====================

NEST is installed with ``cmake`` (at least v2.8.12). In the simplest case, the commands::

    cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> </path/to/NEST/src>
    make
    make install

should build and install NEST to ``/install/path``, which should be an absolute
path.

Choice of CMake Version
------------------------

We recommend to use ``cmake`` v3.4 or later, even though installing NEST with
``cmake`` v2.8.12 will in most cases work properly.
For more detailed information please see below: ``Python3 Binding (PyNEST)``

Choice of compiler
------------------

The default compiler for NEST is GNU gcc/g++. Version 7 or higher is required
due to the presence of bugs in earlier versions that prevent the compilation
from succeeding. NEST has also successfully been compiled with Clang 7 and the
IBM XL C++ compiler.

To select a specific compiler, please add the following flags to your ``cmake``
line::

    -DCMAKE_C_COMPILER=<C-compiler> -DCMAKE_CXX_COMPILER=<C++-compiler>

Options for configuring NEST
----------------------------

NEST allows for several configuration options for custom builds:

Change NEST behavior::

    -Dtics_per_ms=[number]     Specify elementary unit of time. [default 1000.0]
    -Dtics_per_step=[number]   Specify resolution. [default 100]
    -Dwith-ps-arrays=[OFF|ON]  Use PS array construction semantics. [default=ON]

Add user modules::

    -Dexternal-modules=[OFF|<list;of;modules>]  External NEST modules to be linked
                                                in, separated by ';'. [default=OFF]

Connect NEST with external projects::

    -Dwith-libneurosim=[OFF|ON|</path/to/libneurosim>]  Request the use of libneurosim.
                                                        Optionally give the directory,
                                                        where libneurosim is installed.
                                                        [default=OFF]
    -Dwith-music=[OFF|ON|</path/to/music>] Request the use of MUSIC. Optionally
                                           give the directory, where MUSIC is installed.
                                           [default=OFF]

Change parallelization scheme::

    -Dwith-mpi=[OFF|ON|</path/to/mpi>]  Request compilation with MPI. Optionally
                                        give directory with MPI installation.
                                        [default=OFF]
    -Dwith-openmp=[OFF|ON|<OpenMP-Flag>]  Enable OpenMP multi-threading.
                                          Optional: set OMP flag. [default=ON]

Set default libraries::

    -Dwith-gsl=[OFF|ON|</path/to/gsl>]           Find a gsl library. To set a specific
                                                 library, set install path.[default=ON]
    -Dwith-readline=[OFF|ON|</path/to/readline>] Find a GNU Readline library. To set
                                                 a specific library, set install path.
                                                 [default=ON]
    -Dwith-ltdl=[OFF|ON|</path/to/ltdl>]         Find an ltdl library. To set a specific
                                                 ltdl, set install path. NEST uses the
                                                 ltdl for dynamic loading of external
                                                 user modules. [default=ON]
    -Dwith-python=[OFF|ON|2|3]                   Build PyNEST. To set a specific Python
                                                 version, set 2 or 3. [default=ON]
    -Dcythonize-pynest=[OFF|ON]                  Use Cython to cythonize pynestkernel.pyx.
                                                 If OFF, PyNEST has to be build from
                                                 a pre-cythonized pynestkernel.pyx.
                                                 [default=ON]

Change compilation behavior::

    -Dstatic-libraries=[OFF|ON]     Build static executable and libraries. [default=OFF]
    -Dwith-optimize=[OFF|ON|<list;of;flags>]       Enable user defined optimizations. Separate
                                                   multiple flags by ';'.
                                                   [default OFF, when ON, defaults to '-O3']
    -Dwith-warning=[OFF|ON|<list;of;flags>]        Enable user defined warnings. Separate
                                                   multiple flags by ';'.
                                                   [default ON, when ON, defaults to '-Wall']
    -Dwith-debug=[OFF|ON|<list;of;flags>]          Enable user defined debug flags. Separate
                                                   multiple flags by ';'.
                                                   [default OFF, when ON, defaults to '-g']
    -Dwith-intel-compiler-flags=[<list;of;flags>]  User defined flags for the Intel compiler.
                                                   Separate multiple flags by ';'.
                                                   [defaults to '-fp-model strict']
    -Dwith-libraries=<list;of;libraries>           Link additional libraries. Give full path.
                                                   Separate multiple libraries by ';'.
                                                   [default OFF]
    -Dwith-includes=<list;of;includes>             Add additional include paths. Give full
                                                   path without '-I'. Separate multiple include
                                                   paths by ';'. [default OFF]
    -Dwith-defines=<list;of;defines>               Additional defines, e.g. '-DXYZ=1'.
                                                   Separate multiple defines by ';'. [default OFF]

NO-DOC option
--------------

On systems where help extraction is slow, the call to ``make install`` can be replaced
by ``make install-nodoc`` to skip the generation of help pages and indices. Using this
option can help developers to speed up development cycles, but is not recommended for
production use as it renders the built-in help system useless.


Configuring NEST for Distributed Simulation with MPI
--------------------------------------------------------

  1. Try ``-Dwith-mpi=ON`` as argument for ``cmake``. If it works, fine.
  2. If 1 does not work, or you want to use a non-standard MPI,
     try ``-Dwith-mpi=/path/to/my/mpi``.
     Directory mpi should contain include, lib, bin subdirectories for MPI.
  3. If that does not work, but you know the correct compiler wrapper for
     your machine, try configure ``-DMPI_CXX_COMPILER=myC++_CompilerWrapper
     -DMPI_C_COMPILER=myC_CompilerWrapper -Dwith-mpi=ON``
  4. Sorry, you need to fix your MPI installation.

Tell NEST about your MPI setup
------------------------------

If you compiled NEST with support for distributed computing via MPI, you
have to tell it how your ``mpirun``/``mpiexec`` command works by
defining the function mpirun in your ``~/.nestrc`` file. This file
already contains an example implementation that should work with
`OpenMPI <http://www.openmpi.org>`__ library.


Disabling the Python Bindings (PyNEST)
----------------------------------------

To disable Python bindings use::

    -Dwith-python=OFF

as an argument to ``cmake``.

Please see also the file :doc:`../../pynest/README.md` in the documentation directory for details.

Python3 Binding (PyNEST)
--------------------------

To force a Python3-binding in a mixed Python2/3 environment pass::

    -Dwith-python=3

as an argument to ``cmake``.

``cmake`` usually autodetects your Python installation.
In some cases ``cmake`` might not be able to localize the Python interpreter
and its corresponding libraries correctly. To circumvent such a problem following
``cmake`` built-in variables can be set manually and passed to ``cmake``::

  PYTHON_EXECUTABLE ..... path to the Python interpreter
  PYTHON_LIBRARY ........ path to libpython
  PYTHON_INCLUDE_DIR .... two include ...
  PYTHON_INCLUDE_DIR2 ... directories

 e.g.: Please note ``-Dwith-python=ON`` is the default::
  cmake -DCMAKE_INSTALL_PREFIX=</install/path> \
        -DPYTHON_EXECUTABLE=/usr/bin/python3 \
        -DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.4m.so \
        -DPYTHON_INCLUDE_DIR=/usr/include/python3.4 \
        -DPYTHON_INCLUDE_DIR2=/usr/include/x86_64-linux-gnu/python3.4m \
        </path/to/NEST/src>

Compiling for Apple OSX/macOS
=============================

NEST can currently not be compiled with the clang/clang++ compilers shipping
with macOS. Therefore, you first need to install GCC 6.3 or later. The easiest
way to install all required software is using Homebrew (from http://brew.sh)::

  brew install gcc cmake gsl open-mpi libtool

will install all required prequisites. You can then configure NEST with ::

  cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
        -DCMAKE_C_COMPILER=gcc-6\
        -DCMAKE_CXX_COMPILER=g++-6 \
        </path/to/NEST/src>

For detailed information on installing NEST under OSX/macOS, please see the
"macOS" section of https://www.nest-simulator.org/installation.

Choice of compiler
------------------

Most NEST developers use the GNU gcc/g++ compilers. We also regularly compile NEST using the IBM xlc/xlC compilers. You can find the version of your compiler by, e.g.::

    g++ -v

To select a specific compiler, please add the following flags to your ``cmake``
line::

    -DCMAKE_C_COMPILER=<C-compiler> -DCMAKE_CXX_COMPILER=<C++-compiler>


Compiler-specific options
~~~~~~~~~~~~~~~~~~~~~~~~~~~

NEST has reasonable default compiler options for the most common compilers.

When compiling with the *Portland* compiler:
  Use the ``-Kieee`` flag to ensure that computations obey the IEEE754 standard for floating point numerics.

When compiling with the *Intel* compiler:
  To ensure that computations obey the IEEE754 standard for floating point
  numerics, the ``-fp-model strict`` flag is used by default, but can be
  overridden with ::

      -Dwith-intel-compiler-flags="<intel-flags>"
