
Installation instructions for NEST 2.10 and earlier
====================================================

.. note:: These instructions contain only information on points that differ from
 current versions of NEST. :doc:`See the installation page for details <installation>`.

The following are the basic steps to compile and install NEST from source code:

*  :doc:`Download NEST <../download.rst>`

* Unpack the tarball::

    tar -xzvf nest-simulator-x.y.z.tar.gz

* Create a build directory::

    mkdir nest-simulator-x.y.z-build

* Change to the build directory::

    cd nest-simulator-x.y.z-build

* Configure NEST::

    ../nest-simulator-x.y.z/configure

with appropriate configuration options

* Compile and install by running::

    make
    make install

Dependencies
~~~~~~~~~~~~~

The `GNU readline library <http://www.gnu.org/software/readline/>`_ is recommended
if you use NEST interactively without Python. Although most Linux distributions
have GNU readline installed, you still need to install its development package
if want to use GNU readline with NEST. GNU readline itself depends on
`libncurses <http://www.gnu.org/software/ncurses/>`_  <or libtermcap on older
systems). Again, the development packages are needed to compile NEST.

The `GNU Scientific Library <http://www.gnu.org/software/gsl/>`_ is needed by
several neuron models, in particular those with conductance based synapses. If
you want these models, please install the GNU Scientific Library along with its
development packages. We strongly recommend that you install/update to GNU
Scientific Library v1.11 or later.

If you want to use PyNEST, we recommend to install the following along with
their development packages:

- `Python <http://www.python.org>`_
- `NumPy <http://www.scipy.org>`_
- `SciPy <http://www.scipy.org>`_
- `matplotlib <http://matplotlib.org>`_
- `IPython <http://ipython.org>`_

Additionally, NEST depends on a few POSIX libraries which are usually present
on all UNIX like operating systems (Linux, Mac OS), so don't worry. But if you
wonder why NEST is difficult to compile on Windows, it is because of these:

- libpthread, for threading support
- libregexp, for regular expressions.

Minimal configuration
----------------------

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

Standard configuration
-------------------------

To install the packages required for the standard installation use the following
command line::

    sudo apt-get install build-essential autoconf automake libtool libltdl7-dev
    libreadline6-dev libncurses5-dev libgsl0-dev python-all-dev python-numpy
    python-scipy python-matplotlib ipython

Then configure NEST using::

    ../nest-simulator-x.y.z/configure --prefix=$HOME/opt/nest

Configuration options
---------------------------

If you need special features, like e.g. support for :doc:`distributed computing <../guides/parallel_computing>`,
you can add command line switches to the call to configure. ``./configure --help``
will show you all available options. For more information on the installation of
NEST, please see the file INSTALL that is included in the distribution archive
for the version of NEST you want to install.

Known problems for NEST 2.10 and earlier
-----------------------------------------

Using the correct compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We strongly recommend that you use the same compiler version to compile NEST
that was used to build Python / OpenMPI. Usually, this will be the system
compiler (note: by default, MacPorts compiles using the system compiler). Python
displays which compiler it was compiled with at startup.

When compiling NEST with a newer GCC than Python was compiled with, most bets
are off, since different GCC versions generate different binary code. PyNEST may
then crash Python. Either make sure that gcc and g++ are the system compiler,
or force compilation with the system compiler, configure like this (append any
other configure options)::

    ../nest-simulator-x.y.z/configure CC=/usr/bin/gcc CXX=/usr/bin/g++

MPI issues
~~~~~~~~~~~~

Mac OS X 10.5 and later comes with MPI pre-installed and ``--with-mpi`` should
work. If you should get an error message like this::

    libtool: link: /usr/bin/mpicxx -W -Wall -pedantic -Wno-long-long -O0 -g -DNO_UNUSED_SYN -o .libs/nest main.o neststartup.o
    -Wl,-bind_at_load ../models/.libs/libmodelmodule.a /Users/plesser/NEST/code/branches/bluegeneP/bld/topology/.libs/libtopologymodule.dylib
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
    make: *** `nest Error 1

there is most likely a conflict between several MPI installations on your
computer, in the example above with OpenMPI from MacPorts. In this case, you
need to deactivate/uninstall the conflicting MPI installation. It is
unfortunately not trivial to ignore other MPI installations. One brutal
work-around is to edit the ``nest/Makefile``, in the build directory and add
``-L/usr/lib`` before all other ``-L`` options in the line building NEST.
