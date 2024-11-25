.. _mac_install:

Building NEST on macOS
======================

Building NEST on macOS requires some developer tools. There are several sources from
which you can install them, e.g., conda-forge, Homebrew, or MacPorts. The most important
recommendation for an easy and stable build is *not to mix tools from different sources*.
This includes your Python installation: Taking Python from conda-forge and all else from Homebrew
may work, but can also lead to various complications.

This guide shows how to build NEST with a development environment created with mamba. The main
advantage of mamba is that you can fully insulate the entire environment in a mamba environment.
If you want to base your setup on Homebrew or MacPorts, you can still use the
``environment.yml`` file as a guide to necessary packages.

Preparations
------------

1. Install the Xcode command line tools by executing the following line in the terminal and
   following the instructions in the windows that will pop up:

   .. code-block:: sh

      xcode-select --install

#. Create a mamba environment with necessary tools

   .. code:: sh

      cd <nest_source_dir>
      mamba env create -p mamba/

   .. note::

      To build NEST natively on a Mac with Apple's M1 chip, you need to use Miniforge as
      described in :ref:`conda_tips`.

#. Activate the environment with

   .. code:: sh

      mamba activate mamba/

   This assumes that you have created the environment in the folder ``mamba/`` as given above. Note that the trailing
   slash is necessary for mamba to distinguish it from a named environment.

#. If you want to build NEST with MPI, you must digitally sign the ``orterun`` and ``orted`` binaries

   a. If you do not yet have a self-signed code-signing certificate, create one as described here:
      `<https://gcc.gnu.org/onlinedocs/gcc-4.8.1/gnat_ugn_unw/Codesigning-the-Debugger.html>`__
      (restarting does not appear to be necessary any more).
   b. Sign your binaries

      .. code:: sh

         codesign -f -s "gdb-cert" `which orted`
         codesign -f -s "gdb-cert" `which orterun`

      Instead of the ``which`` command you can also give the full path to the binary inside your mamba
      environment.

      .. note::

         You will need to sign the binaries every time you update the OpenMPI package in your environment.


Building NEST
-------------

1. Download or clone the NEST sources from `<https://github.com/nest/nest-simulator>`__.

#. Create a build directory outside the NEST sources and change into it.

#. Double check that you have activated a virtual environment.

#. Configure NEST by running

   .. code-block:: sh

      cmake <nest_source_dir>

   If you have libraries required by NEST such as GSL installed with Homebrew and mamba, this
   can lead to library conflicts (error messages like ``Initializing libomp.dylib, but found
   libomp.dylib already initialized.``). To ensure that libraries are found first in your mamba
   environment, invoke ``cmake`` like this

   .. code-block:: sh

      CMAKE_PREFIX_PATH=<mamba_env_dir> cmake <nest_source_dir>

   You can find the ``<mamba_env_dir>`` for the currently active mamba environment by running
   ``mamba info`` and looking for the "active env location" entry in the output.

   To compile NEST with :ref:`MPI support <distributed_computing>`, add ``-Dwith-mpi=ON`` as ``cmake`` option.
   For further CMake options, see :ref:`cmake_options`.

#. Compile, install, and verify NEST with

   .. code-block:: sh

      make -j4         # -j4 builds in parallel using 4 processes
      make install
      make installcheck

Install NEST outside of a virtual environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default NEST will be installed into the active virtual Python environment. If you wish to
install it elsewhere, you can specify an install prefix. Follow the above instructions, but
use ``cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> <nest_source_dir>`` instead. Note
that when NEST is installed in a non-standard location, automatic discovery of the Python
module is impossible, and environment variables must be set before NEST can be used:

.. code-block:: sh

   source <nest_install_dir>/bin/nest_vars.sh

Troubleshooting
---------------

Mamba with Intel MKL
~~~~~~~~~~~~~~~~~~~~

The Intel Math Kernel Library (MKL) uses a different OpenMP
library to support threading than what's included with Apple Clang or GCC. This will lead
to conflicts if NEST is built with support for threading, which is the default and
usually desirable. One way to avoid this is to follow the instructions above. An
alternative is to create a mamba environment in which you install ``nomkl`` as *the
very first package*. This will tell mamba to install MKL-free versions of NumPy and
other linear-algebra intensive packages.
