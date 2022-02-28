.. _dev_install:

Developer installation instructions
===================================

We encourage developers to install NEST and its dependencies in an environment, such as conda.
We have an `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_
file that contains all possible tools needed for NEST development.

* Clone nest-simulator from Github `<https://github.com/nest/nest-simulator>`_:

.. code-block:: sh

   git clone git@github.com:<your-username>/nest-simulator.git

.. note::

   * For more information on using git see our :ref:`git_workflow`.


or download the tarball `here <https://github.com/nest/nest-simulator/releases>`_ and unpack it:

.. code-block:: sh

    tar -xzvf nest-simulator-x.y.z.tar.gz

* Check out all the :ref:`development workflows here <developer_space>`.



Install from source in a conda environment
------------------------------------------

* Create a conda environment from the `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_ file.
  We recommend specifying a dedicated location ``--prefix <path/to/conda/env>`` for your environment.
  See the `conda documentation <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#specifying-a-location-for-an-environment>`_.
  on using a custom location rather than the default envs folder.

.. code-block:: sh

    conda env create -f nest-simulator/environment.yml --prefix <path/to/conda/env>
    conda activate <path/to/conda/env>

* Create a build directory:

.. code-block:: sh

    mkdir build_dir

* Change to the build directory:

.. code-block:: sh

    cd build_dir

* Configure NEST. Add the cmake option ``-CDMAKE_INSTALL_PREFIX:PATH=$CONDA_PREFIX`` to link nest to your active conda environment.
  You may need additional ``cmake`` options (see :ref:`cmake_options`).

.. code-block:: sh

   cmake -CDMAKE_INSTALL_PREFIX:PATH=$CONDA_PREFIX </path/to/NEST/src>

* Compile and install NEST:

.. code-block:: sh

    make
    make install
    make installcheck

.. note::

   To build the developer or user documentation see :ref:`doc_workflow`

Install from source in a virtual Python environment
----------------------------------------------------

The following are the basic steps to compile and install NEST from source code. See the
:ref:`CMake Options <cmake_options>` or the :ref:`High Performance Computing <hpc_install>` instructions to
further adjust settings for your system.

* If not already installed on your system, the following packages are recommended (see also the `Dependencies`_
  section)

.. code-block:: bash

    sudo apt install -y \
    cython \
    libgsl-dev \
    libltdl-dev \
    libncurses-dev \
    libreadline-dev \
    openmpi-bin \
    libopenmpi-dev

* When NEST is installed with Python and without ``cmake`` option ``-DCMAKE_INSTALL_PREFIX=<nest_install_dir>``,
  only `virtual environments <https://docs.python.org/3/tutorial/venv.html>`_ are supported.
  Activate the virtual environment you want to use, or if you don't already have one, create a new virtual environment for NEST:

.. code-block:: bash

    python -m venv nest_env
    source nest_env/bin/activate

* Unpack the tarball `<missing_link_to_tarball>`_

.. code-block:: sh

    tar -xzvf nest-simulator-x.y.z.tar.gz

* Create a build directory:

.. code-block:: sh

    mkdir nest-simulator-x.y.z-build

* Change to the build directory:

.. code-block:: sh

    cd nest-simulator-x.y.z-build

* Configure NEST. You may need additional ``cmake`` options (see :ref:`cmake_options`).

.. code-block:: sh

   cmake </path/to/NEST/src>

* Compile and install NEST:

.. code-block:: sh

    make
    make install
    make installcheck

NEST should now be successfully installed in your active Python environment.

.. _sec_no_env:

Install from source without a virtual environment
-------------------------------------------------

The following are the basic steps to compile and install NEST from source code. See the
:ref:`CMake Options <cmake_options>` or the :ref:`High Performance Computing <hpc_install>` instructions to
further adjust settings for your system.

* If not already installed on your system, the following packages are recommended.

.. note::

   The complete list of packages for an entire development environment can be found in the environment.yml file
   The list below does not include the requirements for building documentation or running NEST server.

.. code-block:: bash

    sudo apt install -y \
    libtool \
    cmake \ # require >= 3.12
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
    ipython3 \
    python3-future \
    python3-mpi4py \
    openmpi-bin \
    libopenmpi-dev
    libmusic-dev \
    music-bin \
    python3-pip \
    python3-pytest \
    python3-pytest-timeout \
    python3-pytest-xdist


* Create an install directory

.. code-block:: sh

   mkdir nest-simulator-x.y.z-install

We will refer to the full path of this directory by <nest_install_dir>.

* Create a build directory:

.. code-block:: sh

   mkdir nest-simulator-x.y.z-build

* Change to the build directory:

.. code-block:: sh

    cd nest-simulator-x.y.z-build

* Configure NEST. You may need additional ``cmake`` options (see :ref:`cmake_options`).
  Installing NEST with Python outside a virtual Python environment requires the
  ``cmake`` option ``-DCMAKE_INSTALL_PREFIX=<nest_install_dir>``.

.. code-block:: sh

   cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> -DCMAKE_BUILD_TYPE:STRING=Debug </path/to/src/nest-simulator-x.y.z>

.. note::

   ``<nest_install_dir>`` should be an absolute path

.. note::

   Python bindings are enabled by default. Add the configuration option ``-Dwith-python=OFF`` to disable them.

* Compile and install NEST:

.. code-block:: sh

   make
   export PYTHONPATH=<nest_install_dir>/lib/python3.8/site-packages:${PYTHONPATH}
   make install
   PYTHONUSERBASE=<nest_install_dir> pip3 install --user junitparser export PATH=<nest_install_dir>/bin:${PATH}
   make installcheck

For your convenience, a shell script setting all required environment variables is provided in
``<nest_install_dir>/bin/nest_vars.sh``. Setting the environment variables in your active shell session requires
sourcing the script:

.. code-block:: sh

   source <nest_install_dir>/bin/nest_vars.sh


Developer tools
~~~~~~~~~~~~~~~

A complete list of required packages to install NEST, run tests, and  build documentation, is available in the
``environment.yml`` file  on the `NEST simulator GitHub page <https://github.com/nest/nest-simulator>`_.


What gets installed where
-------------------------

By default, everything will be installed to the subdirectories ``<nest_install_dir>/{bin,lib,share}``, where
``/install/path`` is the install path given to ``cmake``:

- Executables ``<nest_install_dir>/bin``
- Dynamic libraries ``<nest_install_dir>/lib/``
- SLI libraries ``<nest_install_dir>/share/nest/sli``
- Documentation ``<nest_install_dir>/share/doc/nest``
- Examples ``<nest_install_dir>/share/doc/nest/examples``
- PyNEST ``<nest_install_dir>/lib/pythonX.Y/site-packages/nest``
- PyNEST examples ``<nest_install_dir>/share/doc/nest/examples/pynest``

If you want to run the ``nest`` executable or use the ``nest`` Python module without providing explicit paths, you
have to add the installation directory to your search paths.
Please refer to the :ref:`next section <environment_variables>` section for this.


