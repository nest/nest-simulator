# Installation

## Introduction

NEST compiles and runs on most Unix-like operating systems including Linux and
Mac OS X. For using NEST on Microsoft Windows, see [below](installation.md#windows).
The installation instructions here should work on all recent versions of Debian
or Ubuntu. For more generic installation instructions, see the files README and
INSTALL in the top source directory.

Following are the basic steps to compile and install NEST from source code:

1.  [Download NEST](download.md)

2.  Unpack the tarball: `tar -xzvf nest-x.y.z.tar.gz`

3.  Create a build directory: `mkdir nest-x.y.z-build`

4.  Change to the build directory: `cd nest-x.y.z-build`

5.  Configure NEST: `../nest-x.y.z/configure` with appropriate
     configuration options (**prefix**, etc.)

6.  Compile by running `make`

7.  Install by running `make install`

8.  See the [Getting started](getting-started.md) pages to
    find out how to get going with NEST

Please see the sections on [minimal](installation.md#minimal-configuration) and
[standard configuration](installation.md#standard-configuration)
below for details.

## What gets installed where

By default, everything will be installed to the subdirectories
`$PREFIX/{bin,lib,share}`, where `$PREFIX=$HOME/opt/nest`:

Executables   
`$PREFIX/bin`

Extensions   
`$PREFIX/lib/nest`

SLI libraries   
`$PREFIX/share/nest/sli`

Documentation   
`$PREFIX/share/doc/nest`

Examples   
`$PREFIX/share/doc/nest/examples`

PyNEST   
`$PREFIX/lib/pythonX.Y/site-packages/nest`

PyNEST examples   
`$PREFIX/share/doc/nest/examples/pynest`

Extras   
`$PREFIX/share/nest/extras/emacs/sli`

You can supply a custom prefix using the `--prefix=$PREFIX` switch for
`./configure`. In this case, you have to add the installation directory to your
search paths. For example, if you are using bash:

    export PATH=$PATH:$PREFIX/bin
    export PYTHONPATH=$PREFIX/lib/pythonX.Y/site-packages:$PYTHONPATH

## Dependencies

NEST needs a few third party tools and libraries to work. On many operating
systems, these can be installed using a *package manager* like `apt`, `port` or
`fink`
(see [Standard configuration](installation.md#standard-configuration)).

If you are building NEST from the SVN sources, you need the GNU autotools
installed on your system to bootstrap the build system. These consist of

-   autoconf
-   automake
-   libtool
-   libltdl

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

-   Python (<http://www.python.org/download>)
-   NumPy (<http://www.scipy.org/Download>)
-   SciPy (<http://www.scipy.org/Download>)
-   matplotlib (<http://matplotlib.sourceforge.net/>)
-   IPython (<http://ipython.scipy.org/moin/Download>)

Additionally, NEST depends on a few POSIX libraries which are usually present
on all UNIX like operating systems (Linux, Mac OS), so don't worry. But if you
wonder why NEST is difficult to compile on Windows, it is because of these:

-   libpthread, for threading support
-   libregexp, for regular expressions.

## Minimal configuration

NEST can be compiled without any external packages; such configuration may be
useful e.g. for initial porting to a new supercomputer. However, this implies
several restrictions: First, some neuron and synapse models will not be
available, as they depend on ODE solvers from the GNU scientific library.
Second, the Python extension will not be available, and third, multi-threading
and parallel computing facilities will be disabled.

To compile NEST without external packages, use the following command line to
configure it:

    tar -xzvf nest-x.y.z.tar.gz

    mkdir nest-x.y.z-build
    cd nest-x.y.z-build

    ../nest-x.y.z/configure
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

    ../nest-x.y.z/configure --prefix=$HOME/opt/nest

## Choice of compiler

**Caveat:** you should compile NEST with the same compiler and version as your
Python. Strange crashes have been observed on importing NEST when compiling
NEST with g++ 4.3 or later on OS X and importing it into Python compiled with
the standard GCC 4.0 compiler. Most NEST developers use the GNU gcc/g++
compilers. We also regularly compile NEST using the Intel icc/icpc compilers
and the IBM xlc/xlC compilers. You can find the version of your compiler by,
e.g.,

    g++ -v

Python displays the compiler used to build it at startup.

### Compiler-specific options

NEST is pre-configured with a reasonable set of compiler options for the most
common compilers.

When compiling with the Portland compiler, use the -Kieee flag to ensure that
computations obey the IEEE754 standard for floating point numerics.

## Configuration options

If you need special features, like e.g. support for [distributed computing](parallel-computing.md),
you can add command line switches to the call to configure. `./configure --help`
will show you all available options. For more information on the installation of
NEST, please see the file INSTALL that is included in the distribution archive.

### Set up the integrated helpdesk

The command `helpdesk` needs to know which browser to launch in order to display
the help pages. The browser is set as an option of `helpdesk`. Please see the
file `~/.nestrc` for an example setting `firefox` as browser. Please note that
the command `helpdesk` does not work if you have compiled NEST with MPI support,
but you have to enter the address of the helpdesk
(`file://$PREFIX/share/doc/nest/index.html`) manually into the browser.
Please replace `$PREFIX` with the prefix you chose during the configuration of
NEST. If you did not explicitly specify one, it is most likely set to `/usr` or
`/usr/local` depending on what system you are.

### Tell NEST about your MPI setup

If you compiled NEST with support for distributed computing via MPI, you have
to tell it how your`mpirun`/`mpiexec` command works by defining the function
mpirun in your `~/.nestrc` file. This file already contains an example
implementation that should work with [OpenMPI](http://www.openmpi.org/) library.

## Mac OS X

The easiest way to install NEST on a Mac is to install it via the Homebrew
package manager:

1.  To install homebrew, follow the instructions here: <http://brew.sh/>

2.  Then, in a terminal

    1.  Add the homebrew/science tap: execute 'brew tap homebrew/science'

    2.  For information on what options NEST has and what will be installed,
        execute 'brew info nest'

    3.  To install nest, execute 'brew install nest'

Options have to be appended, e.g. to install NEST with PyNEST execute
'brew install nest --with-python'.

#Sorry, the following instructions are a little bit outdated!#
A more detailed up-to-date instruction for installation on OSX will
follow soon.

### Installation on OSX 10.9 Mavericks

The clang compiler that ships with OSX 10.9 does not support OpenMP threads.
You therefore need to use GCC to compile NEST. Below you will find instructions
to install all necessary software for building NEST without and with MPI
support.

#### NEST without MPI

1.  Install Xcode from the AppStore.

2.  Install the Xcode command line tools by executing the following line in the
    Terminal and following the instructions in the windows that will pop up

        xcode-select --install

3.  Install the GNU Compiler Collection, the GNU Science Library, and Python
    from [MacPorts](http://en.wikipedia.org/wiki/Macports) (you could use Fink
    or Homebrew, too, but instructions here are based on MacPorts; if you
    have installed a complete Python from another source, e.g. Anaconda,
    you do not need Python from MacPorts):

        sudo port install gcc48                   # GCC compiler collection
        sudo port select gcc mp-gcc48      # make gcc-48 the default compiler
        sudo port install gsl +gcc48           # install GNU Science Library,
        use GCC 4.8 compiler
        sudo port install autoconf automake libtool    # build tools

        # now the Python stuff
        sudo port install python27
        sudo port select python python27   # make MacPorts Python 2.7 the default
        sudo port install py27-cython
        sudo port select cython cython27   # make Python 2.7 python the default cython
        sudo port install py27-numpy py27-scipy py27-matplotlib py27-ipython
        sudo port select ipython ipython27   # set default

4.  Create a directory for building and installing NEST (you should always build
    NEST outside the source code directory; installing NEST in a "place of its
    own" makes it easy to remove NEST later).

5.  Extract the NEST tarball as a subdirectory in that directory, create a
    build-directory next to the source code directory

        cd ~                  # move to home directory
        mkdir NEST       # directory for all NEST stuff
        cd NEST
        mkdir nest22    # directory for all NEST 2.2 stuff
        cd nest22
        tar zxf  nest-2.2.2.tar.gz
        mkdir bld
        cd bld

6.  Configure and build NEST inside that directory:

        ../nest-2.2.2/configure --prefix=<home dir>/NEST/nest22/ins
        make -j4         # -j4 builds in parallel using 4 processes
        make install
        make installcheck

#### NEST with MPI

To compile NEST with MPI, you need to install OpenMPI first. Unfortunately,
recent OpenMPI 1.7 versions do not work with NEST under Mavericks. You therefore
need to trick MacPorts into installing OpenMPI 1.6.4. Proceed as follows:

1.  Perform Steps 1-3 as for NEST without MPI above.

2.  Download the MacPorts [Portfile-openmpi-1.6.4.txt](../../diff/Portfile-openmpi-1.6.4.txt)
    and save it under the name `Portfile` in an arbitrary directory

3.  In Terminal, move to the directory containing `Portfile` and run

        sudo port install +gcc48 +threads configure.compiler=macports-gcc-4.8

4.  Perform steps 4 and 5 as above.

5.  Configure and build NEST as follows

        ../nest-2.2.2/configure CXX=openmpicxx --with-mpi --prefix=<home dir>/NEST/nest22/ins
        make -j4
        make install
        make installcheck

### Installation for Older OSX Versions

#Outdated#

Before you can build NEST under Mac OS X, you need to install Xcode from the
Apple Developer tools installation DVD that came with your Mac (the packages are
called Xcode and Optional on Mac OS X 10.6 Snow Leopard). If you are running
Mac OS X 10.7 Lion or Mac OS X 10.8 Mountain Lion, you need to install Xcode
from Mac App Store or download the DMG images from Apple Developer
Connection and drag Xcode icon to the Applications folder. Proceed to the [ADC login page](http://connect.apple.com)
and enter your ADC ID, then download Xcode 4.3.2 for Lion (or later) and
Command Line Tools for Xcode - Late March 2012 (or later).

#### From a tarball

Decide whether you need GSL and SciPy / IPython, if yes, install them from [MacPorts](http://en.wikipedia.org/wiki/Macports)
or [Fink](http://en.wikipedia.org/wiki/Fink) (command below is for MacPorts and
assumes you are using Python 2.7; if using 2.6, replace `py27` with `py26`
below):

    sudo port install gsl py27-ipython py27-matplotlib py27-scipy

You can save yourself some hassle by making this Python installation the
default, otherwise you will have to specify the right Python to the configure
command:

    sudo port select --set python python27
    sudo port select --set ipython ipython27

Now follow the generic instructions above to install NEST.

#### From Subversion checkout

Install libtool from sources or a package manager, such as Fink or MacPorts,
then run ./bootstrap.sh to bootstrap the build system (create the ./configure
script) and proceed as above.

#### Discussion

To install NEST on Mac OS X you basically have three options:

1.  Use only tools and libraries provided by Apple (compiler, system libraries,
    OpenMPI, Python, NumPy)

2.  Use mostly tools provided by Apple and build the rest from source
    (Pythons etc.)

3.  Use tools provided by third-party package archives (Fink or MacPorts)

Only options (1) and (3) are recommended for non-expert users. Go for (3) if you
are already using either of the package archives mentioned above or intending
to use additional Pythons. Go for (1) if you want to get started immediately
without any extra software (and compile NEST against system OpenMPI, Python &
NumPy, although features requiring GSL / SciPy / matplotlib will not work unless
you install them yourself).

The most convenient source for GSL and NumPy / SciPy / matplotlib is MacPorts
(Fink probably works as well, but we have not tested this). Another source for a
complete Python distribution is the [Enthought Python Distribution](http://www.enthought.com),
but we are not testing against it.

**Note:** as of Mac OS X 10.6 Apple ships glibtool missing libltdl sources,
which means that if you want to go for (1) and not use a tarball, but bootstrap
the build system from e.g. an SVN checkout, you need to install libtool +
libltdl from source (see `extras/install_autotools.sh`).

**Note:** as of Mac OS X 10.7 Apple conveniently no longer provides support for
OpenMPI in the default installation. Additionally, clang became the default
compiler of choice, but it doesn't support OpenMP yet. Therefore, it makes most
sense to install latest GCC, Python, OpenMPI and so on from MacPorts.

### Known problems

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

    ../nest-x.y.z/configure CC=/usr/bin/gcc CXX=/usr/bin/g++

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

#### Compilation issues

If building NEST with PyNEST and you get an error message like this while
compiling PyNEST,

    g++ -arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -g
    -bundle -undefined dynamic_lookup /temp/safari/nest-1.9.8316/pynest/
    build/temp.macosx-10.3-fat-2.6/temp/safari/nest-1.9.8316/pynest/

    [snip]

    nest-1.9.8316/pynest/build/lib.macosx-10.3-fat-2.6/nest/
    pynestkernel.so -Wl,-rpath,/usr/local/lib/nest
    ld: -rpath can only be used when targeting Mac OS X 10.5 or later
    collect2: ld returned 1 exit status
    ld: -rpath can only be used when targeting Mac OS X 10.5 or later
    collect2: ld returned 1 exit status
    lipo: can't open input file: /var/folders/Sg/SgWx-i0h2RWLn++8ZRFtG+++
    +TY/-Tmp-//ccKGzq7b.out (No such file or directory)
    error: command 'g++' failed with exit status 1
    make[1]: *** [all] Error 1
    make: *** [all-recursive] Error 1

please try making NEST as follows:

    env MACOSX_DEPLOYMENT_TARGET=10.5 make

Thanks to Harold Gutch for this hint.

## Windows

Windows and Linux differ considerably. This is the reason why it is difficult
to compile NEST natively under Windows. However, it is still possible to use
NEST under Windows using one of the following methods:

### Using the NEST LiveCD

This is the easiest way to use NEST without having to install anything on your
computer. The LiveCD can be used in almost any computer and boots directly into
a complete Ubuntu system, which already has NEST, Python, and some analysis
tools installed.
The CD image is available on the [download page](download.md).

### Virtual Machines

A virtual machine is a software that lets you use Linux in parallel to Windows.
We recommend to use VirtualBox, which is free software and can be downloaded
from <http://www.virtualbox.org/>. Another popular virtual machine software is <http://www.vmware.com/>.
The easiest way to get started is to download the NEST LiveCD from the [download page](download.md)
and either just boot the CD image directly or install it into a virtual hard
disk.

### Cygwin

Cywin is a software layer which emulates Unix system calls. NEST should compile
and install under Cygwin with the generic installation instructions, but we do
not tests this regularly and do not support NEST under Cygwin at present.
Compilation under Cygwin is very slow, but the execution times are comparable
to an equivalent Linux installation.
