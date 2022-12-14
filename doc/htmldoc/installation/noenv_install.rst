.. _noenv:

Install from source without a virtual environment
=================================================

The following are the basic steps to compile and install NEST from source code. See the
:ref:`CMake Options <cmake_options>` or the :ref:`High Performance Computing <hpc_install>` instructions to
further adjust settings for your system.

* If not already installed on your system, the following packages are recommended.

.. important::

   The list below does not include the requirements for building documentation or running NEST server.
   The complete list of packages for an entire development environment can be found in the `environment.yml
   <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_ file.
   For more information see the :ref:`doc_workflow` and :ref:`nest_server` docs.

.. code-block:: bash

    sudo apt install -y \
    cmake \
    gsl-bin \
    libgsl-dev \
    libboost-dev \
    cython3 \
    libreadline-dev \
    python3-all-dev \
    python3-numpy \
    python3-scipy \
    python3-matplotlib \
    python3-nose \
    python3-junitparser \
    ipython3 \
    python3-future \
    openmpi-bin \
    libopenmpi-dev \
    python3-mpi4py \
    libmusic-dev \
    music-bin \
    python3-pip \
    python3-pytest \
    python3-pytest-timeout \
    python3-pytest-xdist

* Create an install directory

.. code-block:: sh

   mkdir nest-install

We will refer to the full path of this directory by <nest_install_dir>.

* Create a build directory:

.. code-block:: sh

   mkdir nest-build

* Change to the build directory:

.. code-block:: sh

    cd nest-build

* Configure NEST. You may need additional ``cmake`` options (see :ref:`cmake_options`).
  Installing NEST with Python outside a virtual Python environment requires the
  ``cmake`` option ``-DCMAKE_INSTALL_PREFIX=<nest_install_dir>``.

.. code-block:: sh

   cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir>  </path/to/NEST/src/>

.. note::

   ``<nest_install_dir>`` should be an absolute path

.. note::

   Python bindings are enabled by default. Add the configuration option ``-Dwith-python=OFF`` to disable them.

* Compile and install NEST:

.. code-block:: sh

   make
   make install
   make installcheck

For your convenience, a shell script setting all required environment variables is provided in
``<nest_install_dir>/bin/nest_vars.sh``. Setting the environment variables in your active shell session requires
sourcing the script:

.. code-block:: sh

   source <nest_install_dir>/bin/nest_vars.sh



