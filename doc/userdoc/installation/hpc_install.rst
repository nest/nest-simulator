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

Compiling for BlueGene/Q
---------------------------

NEST provides a cmake tool-chain file for cross compilation for BlueGene/Q. When
configuring NEST use the following ``cmake`` line::

    cmake -DCMAKE_TOOLCHAIN_FILE=Platform/BlueGeneQ_XLC \
          -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> \
          -Dwith-python=OFF \
          -Dstatic-libraries=ON \
          </path/to/NEST/src>

If you compile dynamically, be aware that the BlueGene/Q system might not provide an ``ltdl``
library. If you want to dynamically load an external user module, you have to
compile and install an ``ltdl`` yourself and add ``-Dwith-ltdl=<ltdl-install-dir>``
to the ``cmake`` line. Otherwise add ``-Dwith-ltdl=OFF``.

Additionally, the design of ``cmake``'s MPI handling has a broken design, which is
brittle in the case of BGQ and certain libraries (flags to use SIONlib, for example).

If you run into that, you must force ``cmake`` to use the wrappers rather than
it's attempts to extract the proper flags for the underlying compiler
as in::

    -DCMAKE_C_COMPILER=/bgsys/drivers/ppcfloor/comm/xl/bin/mpixlc_r
    -DCMAKE_CXX_COMPILER=/bgsys/drivers/ppcfloor/comm/xl/bin/mpixlcxx_r

BlueGene/Q and PyNEST
~~~~~~~~~~~~~~~~~~~~~

Building PyNEST on BlueGene/Q requires you to compile dynamically, i.e.
``-Dstatic-libraries=OFF``. Furthermore, you have to cythonize the
``pynest/pynestkernel.pyx/.pyx`` on a machine with Cython installed::

    cythonize pynestkernel.pyx

Copy the generated file ``pynestkernel.cpp`` into ``</path/to/NEST/src>/pynest`` on
BlueGene/Q.

CMake <3.4 is buggy when it comes to finding the matching libraries (for many years).
Thus, you also have to specify ``PYTHON_LIBRARY`` and ``PYTHON_INCLUDE_DIR``
if they are not found OR the incorrect libraries are found, e.g.::

 -DPYTHON_LIBRARY=/bgsys/local/python3/3.4.2/lib/libpython3.4m.a
 -DPYTHON_INCLUDE_DIR=/bgsys/local/python3/3.4.2/include/python3.4m

A complete ``cmake`` line for PyNEST could look like this::

    module load gsl

    cmake -DCMAKE_TOOLCHAIN_FILE=Platform/BlueGeneQ_XLC \
      -DCMAKE_INSTALL_PREFIX=<nest_install_dir> \
      -Dstatic-libraries=OFF \
      -Dcythonize-pynest=OFF \
    	  -DCMAKE_C_COMPILER=/bgsys/drivers/ppcfloor/comm/xl/bin/mpixlc_r \
    	  -DCMAKE_CXX_COMPILER=/bgsys/drivers/ppcfloor/comm/xl/bin/mpixlcxx_r \
    	  -DPYTHON_LIBRARY=/bgsys/local/python3/3.4.2/lib/libpython3.4m.a \
    	  -DPYTHON_INCLUDE_DIR=/bgsys/local/python3/3.4.2/include/python3.4m \
      -Dwith-ltdl=OFF \
      <nest-src>

Furthermore, for running PyNEST, make sure all Python dependencies are installed and
environment variables are set properly::

    module load python3/3.4.2

    # adds PyNEST to the PYTHONPATH
    source <nest-install-dir>/bin/nest_vars.sh

    # makes HOME and PYTHONPATH available for Python
    runjob \
      --exp-env HOME \
      --exp-env PATH \
      --exp-env LD_LIBRARY_PATH \
      --exp-env PYTHONUNBUFFERED \
      --exp-env PYTHONPATH \
      ... \
      : /bgsys/local/python3/3.4.2/bin/python3.4 script.py

BlueGene/Q and GCC
~~~~~~~~~~~~~~~~~~~~

Compiling NEST with GCC (``-DCMAKE_TOOLCHAIN_FILE=Platform/BlueGeneQ_GCC``)
might require you to use a GSL library compiled using GCC, otherwise undefined
symbols break your build. After the GSL is built with GCC and installed in
``gsl-install-dir``, add ``-Dwith-gsl=<gsl-install-dir>`` to the ``cmake`` line.

BlueGene/Q and Non-Standard Allocators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use NEST with non-standard allocators on BlueGene/Q (e.g., tcmalloc), you
should compile NEST and the allocator with the same compiler, usually GCC.
Since static linking is recommended on BlueGene/Q, the allocator also needs
to be linked statically. This requires specifying linker flags and the
allocator library as shown in the following example::

     cmake -DCMAKE_TOOLCHAIN_FILE=Platform/BlueGeneQ_GCC \
           -DCMAKE_INSTALL_PREFIX:PATH=$PWD/install \
           -Dstatic-libraries=ON -Dwith-warning=OFF \
           -DCMAKE_EXE_LINKER_FLAGS="-Wl,--allow-multiple-definition" \
           -Dwith-libraries=$HOME/tcmalloc/install/lib/libtcmalloc.a
