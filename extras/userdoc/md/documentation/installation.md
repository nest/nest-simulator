# Installation

These installation instructions are for NEST 2.12 and later as well as the most recent version obtained from [GitHub](https://github.com/nest/nest-simulator). 

Installation instructions for NEST 2.10 and earlier are provided [below](#instructions-for-nest-210-and-earlier).

## Introduction

NEST compiles and runs on most Unix-like operating systems. The installation instructions here should work on recent versions of Ubuntu or Debian, while additional instructions for [OS X/macOS are given below](#mac-os-x). 

For more generic installation instructions, see the `INSTALL` file in the top source directory. For using NEST on Microsoft Windows, see [below](#windows).

Following are the basic steps to compile and install NEST from source code:

1.  [Download NEST](download.md)
1.  Unpack the tarball: `tar -xzvf nest-simulator-x.y.z.tar.gz`
1.  Create a build directory: `mkdir nest-simulator-x.y.z-build`
1.  Change to the build directory: `cd nest-simulator-x.y.z-build`
1.  Configure NEST: `cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> </path/to/NEST/src>` with
additional `cmake` options as needed (see `INSTALL` file; `/install/path` should be an
absolute path)
1.  Compile by running `make`
1.  Install by running `make install`
1.  Run tests by running `make installcheck`
1.  See the [Getting started](getting-started.md) pages to
    find out how to get going with NEST

Please see the sections on [minimal](#minimal-configuration) and [standard configuration](#standard-configuration) below for details.

## What gets installed where

By default, everything will be installed to the subdirectories `/install/path/{bin,lib,share}`, where `/install/path` is the install path given to `cmake`:

- Executables `/install/path/bin`
- Dynamic libraries `/install/path/lib/`
- SLI libraries `/install/path/share/nest/sli`
- Documentation `/install/path/share/doc/nest`
- Examples `/install/path/share/doc/nest/examples`
- PyNEST `/install/path/lib/pythonX.Y/site-packages/nest`
- PyNEST examples `/install/path/share/doc/nest/examples/pynest`
- Extras `/install/path/share/nest/extras/`

If you want to run the `nest` executable or use the `nest` Python module without providing explicit paths, you have to add the installation directory to your search paths. For example, if you are using bash:

    export PATH=$PATH:/install/path/bin
    export PYTHONPATH=/install/path/lib/pythonX.Y/site-packages:$PYTHONPATH

The script `/install/path/bin/nest_vars.sh` can be sourced in `.bashrc` and will set these paths for you. This also allows to switch between NEST installations in a convenient manner.


## Dependencies

NEST needs a few third-party tools and libraries to work. On many operating systems, these can be installed using a *package manager* such as `apt` or `brew` (see [Standard configuration](#standard-configuration)).

To build NEST, you need recent versions of [CMake](https://cmake.org) and [libtool](https://www.gnu.org/software/libtool/libtool.html); the latter should be available for most systems and is probably already installed.

The [GNU readline library](http://www.gnu.org/software/readline/) is recommended if you use NEST interactively without Python. Although most Linux distributions have GNU readline installed, you still need to install its development package if want to use GNU readline with NEST. GNU readline itself depends on [libncurses](http://www.gnu.org/software/ncurses/) (or libtermcap on older systems). Again, the development packages are needed to compile NEST.

The [GNU Scientific Library](http://www.gnu.org/software/gsl/) is needed by several neuron models, in particular those with conductance based synapses. If you want these models, please install the GNU Scientific Library along with its development packages. 

If you want to use PyNEST, we recommend to install the following along with their development packages:

- [Python](http://www.python.org)
- [NumPy](http://www.scipy.org)
- [SciPy](http://www.scipy.org)
- [matplotlib](http://matplotlib.org)
- [IPython](http://ipython.org)


## Minimal configuration

NEST can be compiled without any external packages; such a configuration may be useful e.g. for initial porting to a new supercomputer. However, this implies several restrictions: 

- Some neuron and synapse models will not be available, as they depend on ODE solvers from the GNU Scientific Library.
- The Python extension will not be available, and third, multi-threading and parallel computing facilities will be disabled.

To configure NEST for compilation without external packages, use the following  command line:

    cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
          -Dwith-python=OFF \
          -Dwith-gsl=OFF \
          -Dwith-readline=OFF \
          -Dwith-ltdl=OFF \
          -Dwith-openmp=OFF \
          </path/to/nest/source>
          
## Standard configuration

To install the packages required for the standard installation use the following command line:

    sudo apt-get install -y build-essential cmake libltdl7-dev libreadline6-dev \
    libncurses5-dev libgsl0-dev python-all-dev python-numpy python-scipy \ 
    python-matplotlib ipython openmpi-bin libopenmpi-dev python-nose
    
Then configure NEST using:

    cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
          </path/to/nest/source>
 

## Choice of compiler

Most NEST developers use the GNU gcc/g++ compilers. We also regularly compile NEST using the IBM xlc/xlC compilers. You can find the version of your compiler by, e.g.,

    g++ -v


### Compiler-specific options

NEST has reasonable default compiler options for the most common compilers.

When compiling with the Portland compiler, use the `-Kieee` flag to ensure that computations obey the IEEE754 standard for floating point numerics.

## Configuration options

If you need special features, e.g., support for [distributed computing](parallel-computing.md), please see the `INSTALL` file in the main source code directory for available options.

### Set up the integrated helpdesk

The command `helpdesk` needs to know which browser to launch in order to display the help pages. The browser is set as an option of `helpdesk`. Please see the file `~/.nestrc` for an example setting `firefox` as browser. Please note that the command `helpdesk` does not work if you have compiled NEST with MPI support, but you have to enter the address of the helpdesk (`file:///install/path/share/doc/nest/index.html`) manually into the browser. Please replace `/install/path` with the path under which NEST is installed.

### Tell NEST about your MPI setup

If you compiled NEST with support for distributed computing via MPI, you have to tell it how your`mpirun`/`mpiexec` command works by defining the function mpirun in your `~/.nestrc` file. This file already contains an example implementation that should work with [OpenMPI](http://www.openmpi.org/).

## Mac OS X

On the Mac, you can install NEST either via Homebrew or manually. If you want to use PyNEST, you need to have a version of Python with some science packages installed, see the [section Python on Mac](#python-on-mac) for details. 

### Installation via Homebrew

The easiest way to install NEST on a Mac is to install it via the Homebrew package manager:

1.  To install homebrew, follow the instructions at [brew.sh](http://brew.sh/)

2.  Then, in a terminal

    1.  Add the homebrew/science tap by running 
    
        `brew tap brewsci/science`
    
    2.  For information on what options NEST has and what will be installed,
        run 
        
        `brew info nest`
        
    3.  To install nest, execute 
    
        `brew install nest`

Options have to be appended, e.g. to install NEST with PyNEST run

    brew install nest --with-python
    
This will install the most recent release version of NEST. To build
NEST from the most recent sources on Github, use

    brew install nest --HEAD

### Manual installation

The clang/clang++ compiler that ships with OS X/macOS does not support OpenMP threads and creates code that fails some tests. You therefore need to use GCC to compile NEST under OS X/macOS.

Installation instructions here have been tested under OS X 10.11 *El Capitan* and macOS 10.12 *Sierra* with [Anaconda Python 2 and 3](https://www.continuum.io/anaconda-overview) and all other dependencies installed via [Homebrew](http://brew.sh). See below for information when using MacPorts.

1.  Install Xcode from the AppStore.

2.  Install the Xcode command line tools by executing the following line in the Terminal and following the instructions in the windows that will pop up

        xcode-select --install

3.  Install dependencies via Homebrew

        brew install gcc cmake gsl open-mpi libtool

4.  Create a directory for building and installing NEST (you should always build NEST outside the source code directory; installing NEST in a "place of its own" makes it easy to remove NEST later).

5.  Extract the NEST tarball as a subdirectory in that directory or clone NEST from GitHub into a subdirectory. 

        mkdir NEST       # directory for all NEST stuff
        cd NEST
        tar zxf nest-simulator-x.y.z.tar.gz
        mkdir bld
        cd bld

6.  Configure and build NEST inside the build directory:

        cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
              -DCMAKE_C_COMPILER=gcc-6 \
              -DCMAKE_CXX_COMPILER=g++-6 \
              </path/to/NEST/src>
  
        make -j4         # -j4 builds in parallel using 4 processes
        make install
        make installcheck

To compile NEST with MPI support, add `-Dwith-mpi=ON` as `cmake` option.

#### Manual installation with dependencies from MacPorts

The following should work if you install dependencies using MacPorts (only steps that differ from the instructions above are shown):

3. Install dependencies via MacPorts

        sudo port install gcc6 cmake gsl openmpi-default libtool \
        python27 py27-cython py27-nose doxygen
    
6. Configure and build NEST inside the build directory:

        cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> \
              -DPYTHON_LIBRARY=/opt/local/lib/libpython2.7.dylib \ 
              -DPYTHON_INCLUDE_DIR=/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 \
              -DCMAKE_C_COMPILER=/opt/local/bin/gcc-mp-6 \
              -DCMAKE_CXX_COMPILER=/opt/local/bin/g++-mp-6 \
              </path/to/NEST/src>
  
        make -j4         # -j4 builds in parallel using 4 processes
        make install
        make installcheck

To compile NEST with MPI support, add `-Dwith-mpi=ON` as `cmake` option.


### Python on Mac

The version of Python shipping with OS X/macOS is rather dated and does not include key packages such as NumPy. Therefore, you need to install Python via a channel that provides scientific packages.

One well-tested source is the [Anaconda](https://www.continuum.io/anaconda-overview) Python distribution for both Python 2 and 3. If you do not want to install the full Anaconda distribution, you can also install [Miniconda](http://conda.pydata.org/miniconda.html) and then install the packages needed by NEST by running

        conda install numpy scipy matplotlib ipython cython nose

Alternatively, you should be able to install the necessary Python packages via Homebrew, but this has not been tested.

## Troubleshooting

If your operating system does not find the `nest` executable or Python does not find the `nest` module, your path variables may not be set correctly. This may also be the case if Python cannot load the `nest` module due to missing or incompatible libraries. In this case, please run

        source </path/to/nest_install_dir>/bin/nest_vars.sh

to set the necessary environment variables. You may want to include this line in your `.bashrc` file, so that the environment variables are set automatically.

## Windows

Windows and Linux differ considerably. This is the reason why it is difficult to compile NEST natively under Windows. However, it is still possible to use NEST under Windows using one of the following methods:

### Virtual Machines

A virtual machine is a software that lets you use Linux on top of Windows. Popular virtual machine packages are [VirtualBox](http://www.virtualbox.org) and [VMWare](http://www.vmware.com). Once you have installed a virtual machine package, you can [download OVA images containing NEST](download.md) and import them into your virtual machine. 

### Cygwin

Cywin is a software layer which emulates Unix system calls. NEST should compile and install under Cygwin with the generic installation instructions, but we have not tested this for a long time and do not support NEST under Cygwin at present. Compilation under Cygwin is very slow, but the execution times are comparable to an equivalent Linux installation.


## Instructions for NEST 2.10 and earlier

Note: These instructions contain only information on points that differ from
current versions of NEST.

Following are the basic steps to compile and install NEST from source code:

1.  [Download NEST](download.md)

2.  Unpack the tarball: `tar -xzvf nest-simulator-x.y.z.tar.gz`

3.  Create a build directory: `mkdir nest-simulator-x.y.z-build`

4.  Change to the build directory: `cd nest-simulator-x.y.z-build`

5.  Configure NEST: `../nest-simulator-x.y.z/configure` with appropriate
     configuration options

6.  Compile by running `make`

7.  Install by running `make install`

8.  See the [Getting started](getting-started.md) pages to
    find out how to get going with NEST

## Dependencies

The [GNU readline library](http://www.gnu.org/software/readline/) is recommended
if you use NEST interactively without Python. Although most Linux distributions
have GNU readline installed, you still need to install its development package
if want to use GNU readline with NEST. GNU readline itself depends on
[libncurses](http://www.gnu.org/software/ncurses/) (or libtermcap on older
systems). Again, the development packages are needed to compile NEST.

The [GNU Scientific Library](http://www.gnu.org/software/gsl/) is needed by
several neuron models, in particular those with conductance based synapses. If
you want these models, please install the GNU Scientific Library along with its
development packages. We strongly recommend that you install/update to GNU
Scientific Library v1.11 or later.

If you want to use PyNEST, we recommend to install the following along with
their development packages:

- [Python](http://www.python.org)
- [NumPy](http://www.scipy.org)
- [SciPy](http://www.scipy.org)
- [matplotlib](http://matplotlib.org)
- [IPython](http://ipython.org)

Additionally, NEST depends on a few POSIX libraries which are usually present
on all UNIX like operating systems (Linux, Mac OS), so don't worry. But if you
wonder why NEST is difficult to compile on Windows, it is because of these:

- libpthread, for threading support
- libregexp, for regular expressions.

## Minimal configuration

NEST can be compiled without any external packages; such configuration may be
useful e.g. for initial porting to a new supercomputer. However, this implies
several restrictions: First, some neuron and synapse models will not be
available, as they depend on ODE solvers from the GNU scientific library.
Second, the Python extension will not be available, and third, multi-threading
and parallel computing facilities will be disabled.

To compile NEST without external packages, use the following command line to
configure it:

    tar -xzvf nest-simulator-x.y.z.tar.gz
    mkdir nest-simulator-x.y.z-build
    cd nest-simulator-x.y.z-build

    ../nest-simulator-x.y.z/configure
        --prefix=$HOME/opt/nest
        --without-python
        --without-readline
        --without-pthread

## Standard configuration

To install the packages required for the standard installation use the following
command line:

    sudo apt-get install build-essential autoconf automake libtool libltdl7-dev
    libreadline6-dev libncurses5-dev libgsl0-dev python-all-dev python-numpy
    python-scipy python-matplotlib ipython

Then configure NEST using:

    ../nest-simulator-x.y.z/configure --prefix=$HOME/opt/nest

## Configuration options

If you need special features, like e.g. support for [distributed computing](parallel-computing.md),
you can add command line switches to the call to configure. `./configure --help`
will show you all available options. For more information on the installation of
NEST, please see the file INSTALL that is included in the distribution archive
for the version of NEST you want to install.

### Known problems for NEST 2.10 and earlier

####  Using the correct compiler

We strongly recommend that you use the same compiler version to compile NEST
that was used to build Python / OpenMPI. Usually, this will be the system
compiler (note: by default, MacPorts compiles using the system compiler). Python
displays which compiler it was compiled with at startup.

When compiling NEST with a newer GCC than Python was compiled with, most bets
are off, since different GCC versions generate different binary code. PyNEST may
then crash Python. Either make sure that gcc and g++ are the system compiler,
or force compilation with the system compiler, configure like this (append any
other configure options):

    ../nest-simulator-x.y.z/configure CC=/usr/bin/gcc CXX=/usr/bin/g++

#### MPI issues

Mac OS X 10.5 and later comes with MPI pre-installed and `--with-mpi` should
work. If you should get an error message like this

    libtool: link: /usr/bin/mpicxx -W -Wall -pedantic -Wno-long-long -O0 -g -DNO_UNUSED_SYN -o .libs/nest main.o neststartup.o
    -Wl,-bind_at_load   ../models/.libs/libmodelmodule.a ../precise/.libs/libprecisemodule.a
    /Users/plesser/NEST/code/branches/bluegeneP/bld/topology/.lib s/libtopologymodule.dylib
    /Users/plesser/NEST/code/branches/bluegeneP/bld/developer/.libs/libdevelopermodule.dylib ../nestkernel/.libs/libnest.a
    ../librandom/.libs /librandom.a ../libnestutil/.libs/libnestutil.a ../sli/.libs/libsli.a -lcurses -lreadline -lpthread -L/opt/local/lib  
    /opt/local/lib/libgsl.dylib /opt/local/lib/libgslcblas.dylib -lm
    Undefined symbols:
     "MPI::Comm::Set_errhandler(MPI::Errhandler const&)", referenced from:
         vtable for MPI::Commin main.o
         vtable for MPI::Intracommin main.o
         vtable for MPI::Cartcommin main.o

    [snip]

         vtable for MPI::Winin libnestutil.a(libnestutil_la-nest_timemodifier.o)
         vtable for MPI::Winin libnest.a(libnest_la-ring_buffer.o)
    ld: symbol(s) not found
    collect2: ld returned 1 exit status
    make: *** [nest] Error 1

there is most likely a conflict between several MPI installations on your
computer, in the example above with OpenMPI from MacPorts. In this case, you
need to deactivate/uninstall the conflicting MPI installation. It is
unfortunately not trivial to ignore other MPI installations. One brutal
work-around is to edit the `nest/Makefile,` in the build directory and add
`-L/usr/lib` before all other `-L` options in the line building NEST.
