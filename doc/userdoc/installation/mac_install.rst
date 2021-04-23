Building NEST on macOS
======================

Building NEST on macOS requires some developer tools. There are several sources from
which you can install them, e.g., Conda, Homebrew, or MacPorts. The most important
recommendation for an easy and stable build is *not to mix tools from different sources*.
This includes your Python installation: Taking Python from Conda and all else from Homebrew
may work, but can also lead to various complications.

This guide shows how to build NEST with a development environment created with Conda. The main
advantage of Conda is that you can fully insulate the entire environment in a Conda environment.
If you want to base your setup on Homebrew or MacPorts, you can still use the 
``extras/conda-nest-simulator-dev.yml`` file as a guide to necessary packages.

Preparations
------------

1. Install the Xcode command line tools by executing the following line in the terminal and 
   following the instructions in the windows that will pop up:

   .. code-block:: sh

      xcode-select --install

#. Create a conda environment with necessary tools (see also :doc:`conda_tips`)

   .. code:: sh

      conda env create -f extras/conda-nest-simulator-dev.yml

   .. note::

      To build NEST natively on a Mac with Apple's M1 chip, you need to use Miniforge as 
      described in :doc:`conda_tips`.

#. Activate the environment with

   .. code:: sh
   
      conda acvitate nest-simulator
      
   This assumes that you have created the environment with its default name ``nest-simulator``.

#. If you want to build NEST with MPI, you must digitally sign the ``orterun`` and ``orted`` binaries

   a. If you do not yet have a self-signed code-signing certificate, create one as described here:
      `<https://gcc.gnu.org/onlinedocs/gcc-4.8.1/gnat_ugn_unw/Codesigning-the-Debugger.html>`__
      (restarting does not appear to be necessary any more).
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

#. Create a build directory outside the NEST sources and change into it.

#. Configure NEST by running

   .. code-block:: sh

      cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> <nest_source_dir>

   If you have libraries required by NEST such as GSL installed with Homebrew and Conda, this
   can lead to library conflicts (error messages like ``Initializing libomp.dylib, but found
   libomp.dylib already initialized.``). To ensure that libraries are found first in your conda
   environment, invoke ``cmake`` like this

   .. code-block:: sh

      CMAKE_PREFIX_PATH=<conda_env_dir> cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> <nest_install_dir>

   You can find the ``<conda_env_dir>`` for the currently active conda environment by running
   ``conda info`` and looking for the "active env location" entry in the output.

#. Compile, install, and verify NEST with

   .. code-block:: sh

      make -j4         # -j4 builds in parallel using 4 processes
      make install
      make installcheck

   To compile NEST with :ref:`MPI support <distributed-computing>`, add ``-Dwith-mpi=ON`` as ``cmake`` option.
   For further CMake options, see :doc:`cmake_options`.

#. To run NEST, configure your environment with

   .. code-block:: sh

      source <nest_install_dir>/bin/nest_vars.sh


Troubleshooting
---------------

Conda with Intel MKL
~~~~~~~~~~~~~~~~~~~~

A default installation of Anaconda or Miniconda will install a version of NumPy
built on the Intel Math Kernel Library (MKL). This library uses a different OpenMP
library to support threading than what's included with Apple Clang or GCC. This will lead
to conflicts if NEST is built with support for threading, which is the default and
usually desirable. One way to avoid this is to follow the instructions above. An
alternative is to create a conda environment in which you install ``nomkl`` as *the
very first package*. This will tell conda to install MKL-free versions of NumPy and
other linear-algebra intensive packages.
