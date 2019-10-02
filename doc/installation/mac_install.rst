Manual Installation on macOS
===============================

If you want to use PyNEST, you need to have a version of Python with some science packages installed, see the `section Python on Mac <python-on-mac>`_ for details.

The clang/clang++ compiler that ships with OS X/macOS does not support OpenMP threads and creates code that fails some tests. You therefore need to use **GCC** to compile NEST under OS X/macOS.

Installation instructions here have been tested under macOS 10.14 *Mojave* with `Anaconda Python 3 <https://www.continuum.io/anaconda-overview>`_ and all other dependencies installed via `Homebrew <http://brew.sh>`_. They should also work with earlier versions of macOS.

#.  Install Xcode from the AppStore.

#.  Install the Xcode command line tools by executing the following line in the terminal and following the instructions in the windows that will pop up:

 .. code-block:: sh

        xcode-select --install

#.  Install dependencies via Homebrew:

 .. code-block:: sh

       brew install gcc cmake gsl open-mpi libtool

#.  Create a directory for building and installing NEST (you should always build NEST outside the source code directory; installing NEST in a "place of its own" makes it easy to remove NEST later).

#.  Extract the NEST tarball as a subdirectory in that directory or clone NEST from GitHub into a subdirectory:

 .. code-block:: sh

        mkdir NEST       # directory for all NEST stuff
        cd NEST
        tar zxf nest-simulator-x.y.z.tar.gz
        mkdir bld
        cd bld

#.  Configure and build NEST inside the build directory (replacing `gcc-9` and `g++-9` with the GCC  compiler versions you installed with `brew`):

 .. code-block:: sh

        cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
              -DCMAKE_C_COMPILER=gcc-9 \
              -DCMAKE_CXX_COMPILER=g++-9 \
              </path/to/NEST/src>

 .. code-block:: sh

        make -j4         # -j4 builds in parallel using 4 processes
        make install
        make installcheck

To compile NEST with MPI support, add ``-Dwith-mpi=ON`` as ``cmake`` option.

Troubleshooting
.................

If compiling NEST as described above fails with an error message like

 .. code-block:: sh
 
        In file included from /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/sys/wait.h:110,
                         from /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/stdlib.h:66,
                         from /usr/local/Cellar/gcc/9.2.0/include/c++/9.2.0/cstdlib:75,
                         from /usr/local/Cellar/gcc/9.2.0/include/c++/9.2.0/bits/stl_algo.h:59,
                         from /usr/local/Cellar/gcc/9.2.0/include/c++/9.2.0/algorithm:62,
                         from /Users/plesser/NEST/code/src/sli/dictutils.h:27,
                         from /Users/plesser/NEST/code/src/sli/dictutils.cc:23:
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/sys/resource.h:443:34: error: expected initializer before '__OSX_AVAILABLE_STARTING'
          443 | int     getiopolicy_np(int, int) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
              |                                  ^~~~~~~~~~~~~~~~~~~~~~~~
 
 you most likely have installed a version of XCode prepared for the next version of macOS. You can attempt to fix this by running
 
  .. code-block:: sh
  
          sudo xcode-select -s /Library/Developer/CommandLineTools/
          
If this does not help, you can reset to the default XCode path using

  .. code-block:: sh
  
          sudo xcode-select -r

 


Python on Mac
--------------

The version of Python shipping with OS X/macOS is rather dated and does not include key packages such as NumPy. Therefore, you need to install Python via a channel that provides scientific packages.

One well-tested source is the `Anaconda <https://www.continuum.io/anaconda-overview>`_ Python distribution for both Python 2 and 3. If you do not want to install the full Anaconda distribution, you can also install `Miniconda <http://conda.pydata.org/miniconda.html>`_ and then install the packages needed by NEST by running::

        conda install numpy scipy matplotlib ipython cython nose

Alternatively, you should be able to install the necessary Python packages via Homebrew, but this has not been tested.
