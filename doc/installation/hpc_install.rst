High Performance Computer Systems Installation
================================================

Minimal configuration
-------------------------

NEST can be compiled without any external packages; such a configuration may be useful for initial porting to a new supercomputer. However, this implies several restrictions: 

- Some neuron and synapse models will not be available, as they depend on ODE solvers from the GNU Scientific Library.
- The Python extension will not be available
- Multi-threading and parallel computing facilities will be disabled.

To configure NEST for compilation without external packages, use the following  command::

    cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
          -Dwith-python=OFF \
          -Dwith-gsl=OFF \
          -Dwith-readline=OFF \
          -Dwith-ltdl=OFF \
          -Dwith-openmp=OFF \
          </path/to/nest/source>

See the :doc:`Configuration Options <install_options>` to  further adjust settings for your system.

Compiling for BlueGene/Q
---------------------------

NEST provides a cmake tool-chain file for cross compilation for BlueGene/Q. When
configuring NEST use the following ``cmake`` line::

    cmake -DCMAKE_TOOLCHAIN_FILE=Platform/BlueGeneQ_XLC \
          -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
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
BlueGene/Q and point ``-Dwith-python=<...>`` to a valid python version for cross
compilation, either Python 2::

    -Dwith-python=/bgsys/tools/Python-2.7/bin/hostpython

or (much better) Python 3::

    -Dwith-python=/bgsys/local/python3/3.4.2/bin/python3

CMake <3.4 is buggy when it comes to finding the matching libraries (for many years).
Thus, you also have to specify ``PYTHON_LIBRARY`` and ``PYTHON_INCLUDE_DIR``
if they are not found OR the incorrect libraries are found, e.g.::

 -DPYTHON_LIBRARY=/bgsys/tools/Python-2.7/lib64/libpython2.7.so.1.0
 -DPYTHON_INCLUDE_DIR=/bgsys/tools/Python-2.7/include/python2.7

or (much better)::

 -DPYTHON_LIBRARY=/bgsys/local/python3/3.4.2/lib/libpython3.4m.a
 -DPYTHON_INCLUDE_DIR=/bgsys/local/python3/3.4.2/include/python3.4m

A complete ``cmake`` line for PyNEST could look like this::

    module load gsl

    cmake -DCMAKE_TOOLCHAIN_FILE=Platform/BlueGeneQ_XLC \
      -DCMAKE_INSTALL_PREFIX=</install/path> \
      -Dstatic-libraries=OFF \
      -Dcythonize-pynest=OFF \
    	  -DCMAKE_C_COMPILER=/bgsys/drivers/ppcfloor/comm/xl/bin/mpixlc_r \
    	  -DCMAKE_CXX_COMPILER=/bgsys/drivers/ppcfloor/comm/xl/bin/mpixlcxx_r \
    	  -Dwith-python=/bgsys/local/python3/3.4.2/bin/python3 \
    	  -DPYTHON_LIBRARY=/bgsys/local/python3/3.4.2/lib/libpython3.4m.a \
    	  -DPYTHON_INCLUDE_DIR=/bgsys/local/python3/3.4.2/include/python3.4m \
      -Dwith-ltdl=OFF \
      <nest-src>

Furthermore, for running PyNEST, make sure all python dependencies are installed and
environment variables are set properly::

    module load python3/3.4.2
    
    # adds PyNEST to the PYTHONPATH
    source <nest-install-dir>/bin/nest_vars.sh
    
    # makes HOME and PYTHONPATH available for python
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
     

Compiling for Fujitsu Sparc64
-------------------------------

On the K Computer:
  The preinstalled ``cmake`` version is 2.6, which is too old for NEST. Please install
  a newer version, for example::

      wget https://cmake.org/files/v3.4/cmake-3.4.2.tar.gz
      tar -xzf cmake-3.4.2.tar.gz
      mv cmake-3.4.2 cmake.src
      mkdir cmake.build
      cd cmake.build
      ../cmake.src/bootstrap --prefix=$PWD/install --parallel=4
      gmake -j4
      gmake install

  Also you might need a cross compiled GNU Scientific Library (GSL). For GSL 2.1
  this is a possible installation scenario::

      wget ftp://ftp.gnu.org/gnu/gsl/gsl-2.1.tar.gz
      tar -xzf gsl-2.1.tar.gz
      mkdir gsl-2.1.build gsl-2.1.install
      cd gsl-2.1.build
      ../gsl-2.1/configure --prefix=$PWD/../gsl-2.1.install/ \
                           CC=mpifccpx \
                           CXX=mpiFCCpx \
                           CFLAGS="-Nnoline" \
                           CXXFLAGS="--alternative_tokens -O3 -Kfast,openmp, -Nnoline, -Nquickdbg -NRtrap" \
                           --host=sparc64-unknown-linux-gnu \
                           --build=x86_64-unknown-linux-gnu
      gmake -j4
      gmake install

  To install NEST, use the following ``cmake`` line::

      cmake -DCMAKE_TOOLCHAIN_FILE=Platform/Fujitsu-Sparc64 \
            -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
            -Dwith-gsl=/path/to/gsl-2.1.install/ \
            -Dwith-optimize="-Kfast" \
            -Dwith-defines="-DUSE_PMA" \
            -Dwith-python=OFF \
            -Dwith-warning=OFF \
            </path/to/NEST/src>
      make -j4
      make install

  The compilation can take quite some time compiling the file ``models/modelsmodule.cpp``
  due to generation of many template classes. To speed up the process, you can
  comment out all synapse models you do not need.
  The option ``-Kfast`` on the K computer enables many different options::

        -O3 -Kdalign,eval,fast_matmul,fp_contract,fp_relaxed,ilfunc,lib,mfunc,ns,omitfp,prefetch_conditional,rdconv -x-

  Be aware that, with the option ``-Kfast`` an internal compiler error - probably
  an out of memory situation - can occur. One solution is to disable synapse
  models that you don't use in ``models/modelsmodule.cpp``. From current observations
  this might be related to the ``-x-`` option; you can give it a fixed value, e.g
  ``-x1``, and the compilation succeeds (the impact on performance was not analyzed)::

        -Dwith-optimize="-Kfast -x1"
