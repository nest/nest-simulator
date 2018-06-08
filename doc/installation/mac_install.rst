Installation on MAC OS X
=========================


On the Mac, you can install NEST either via Homebrew or manually. If you want to use PyNEST, you need to have a version of Python with some science packages installed, see the `section Python on Mac <python-on-mac>`_ for details. 

Installation via Homebrew
--------------------------

The easiest way to install NEST on a Mac is to install it via the Homebrew package manager:

*  To install homebrew, follow the instructions at `brew.sh <http://brew.sh/>`_

*  Then, in a terminal

    *  Add the homebrew/science tap by running:: 

        brew tap brewsci/science

    *  For information on what options NEST has and what will be installed, run::

        brew info nest

    *  To install nest, execute:: 

        brew install nest

Options have to be appended, so for example, to install NEST with PyNEST run::

    brew install nest --with-python

This will install the most recent release version of NEST. To build
NEST from the most recent sources on Github, use::

    brew install nest --HEAD

Manual installation
--------------------

The clang/clang++ compiler that ships with OS X/macOS does not support OpenMP threads and creates code that fails some tests. You therefore need to use **GCC** to compile NEST under OS X/macOS.

Installation instructions here have been tested under OS X 10.11 *El Capitan* and macOS 10.12 *Sierra* with `Anaconda Python 2 and 3 <https://www.continuum.io/anaconda-overview>`_ and all other dependencies installed via `Homebrew <http://brew.sh>`_. See below for `Manual installation with dependencies from MacPorts`_.

*  Install Xcode from the AppStore.

*  Install the Xcode command line tools by executing the following line in the terminal and following the instructions in the windows that will pop up::

        xcode-select --install

*  Install dependencies via Homebrew::

        brew install gcc cmake gsl open-mpi libtool
*  Create a directory for building and installing NEST (you should always build NEST outside the source code directory; installing NEST in a "place of its own" makes it easy to remove NEST later).

*  Extract the NEST tarball as a subdirectory in that directory or clone NEST from GitHub into a subdirectory::

        mkdir NEST       # directory for all NEST stuff
        cd NEST
        tar zxf nest-simulator-x.y.z.tar.gz
        mkdir bld
        cd bld

*  Configure and build NEST inside the build directory::

        cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
              -DCMAKE_C_COMPILER=gcc-6 \
              -DCMAKE_CXX_COMPILER=g++-6 \
              </path/to/NEST/src>

  .. code-block:: bash
  
        make -j4         # -j4 builds in parallel using 4 processes
        make install
        make installcheck

To compile NEST with MPI support, add ``-Dwith-mpi=ON`` as ``cmake`` option.

Manual installation with dependencies from MacPorts
"""""""""""""""""""""""""""""""""""""""""""""""""""

The following should work if you install dependencies using MacPorts (only steps that differ from the instructions above are shown):

* Install dependencies via MacPorts::

        sudo port install gcc6 cmake gsl openmpi-default libtool \
        python27 py27-cython py27-nose doxygen

* Configure and build NEST inside the build directory::

        cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
              -DPYTHON_LIBRARY=/opt/local/lib/libpython2.7.dylib \ 
              -DPYTHON_INCLUDE_DIR=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 \
              -DCMAKE_C_COMPILER=/opt/local/bin/gcc-mp-6 \
              -DCMAKE_CXX_COMPILER=/opt/local/bin/g++-mp-6 \
              </path/to/NEST/src>

  .. code-block:: bash

        make -j4         # -j4 builds in parallel using 4 processes
        make install
        make installcheck

To compile NEST with MPI support, add ``-Dwith-mpi=ON`` as ``cmake`` option.


Python on Mac
--------------

The version of Python shipping with OS X/macOS is rather dated and does not include key packages such as NumPy. Therefore, you need to install Python via a channel that provides scientific packages.

One well-tested source is the `Anaconda <https://www.continuum.io/anaconda-overview>`_ Python distribution for both Python 2 and 3. If you do not want to install the full Anaconda distribution, you can also install `Miniconda <http://conda.pydata.org/miniconda.html>`_ and then install the packages needed by NEST by running::

        conda install numpy scipy matplotlib ipython cython nose

Alternatively, you should be able to install the necessary Python packages via Homebrew, but this has not been tested.

