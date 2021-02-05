Building NEST on macOS
======================

Building NEST on macOS requires some developer tools. There are several sources from
which you can install them, e.g., Conda, Homebrew or MacPorts. The most important
recommendation for an easy and stable build is *not to mix tools from different sources*.
This includes your Python installation: Taking Python from Conda and all else from Homebrew
may work, but can also lead to various complications.

This guide shows how to build NEST with a development environment created with Conda. The main
advantage of Conda is that you can fully insulate the entire environment in a Conda environment.
If you want to base your setup on Homebrew or MacPorts, you can still use the ``extras/conda-env-nest-simulator.yml``
file as a guide to necessary packages.

Preparations
------------

1. Install the Xcode command line tools by executing the following line in the terminal and 
   following the instructions in the windows that will pop up:

   .. code-block:: sh

      xcode-select --install

#. Create a conda environment with necessary tools (see also :doc:`conda_tips`)

   .. code:: sh

      conda env create -f extras/conda-environment-nest-simulator.yml

   .. note::

      To build NEST natively on a Mac with Apple's M1 chip, you need to use Miniforge as 
      described in :doc:`conda_tips`.

#. Activate the environment.

#. If you want to build NEST with MPI, you must codesign the ``orterun`` and ``orted`` binaries

   a. If you do not yet have a self-signed code-signing certificate, create one as described here:
      `<https://gcc.gnu.org/onlinedocs/gcc-4.8.1/gnat_ugn_unw/Codesigning-the-Debugger.html>`__.
   b. Sign your binaries

      .. code:: sh

         codesign -f -s "gdb-cert" `which orted`
         codesign -f -s "gdb-cert" `which orterun`

      Instead of the ``which`` command you can also give the full path to the binary inside your conda
      environment.
      
      .. note::
      
         You will need to sign the binaries every time you update the OpenMPI package in your environment.


Building NEST
-------------

1. Download or clone the NEST sources from `<https://github.com/nest/nest-simulator>`__.

#. Create a build directory outside the NEST sources and move into it.

#. Configure NEST by running

   .. code-block:: sh

      cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> </path/to/NEST/src>

#. Compile, install and verify NEST with

   .. code-block:: sh

      make -j4         # -j4 builds in parallel using 4 processes
      make install
      make installcheck

   To compile NEST with MPI support, add ``-Dwith-mpi=ON`` as ``cmake`` option.
   For further CMake options, see :doc:`compilation_options`.

#. To run NEST, configure your environment with

   .. code-block:: sh

      source <nest_install_dir>/bin/nest_vars.sh
