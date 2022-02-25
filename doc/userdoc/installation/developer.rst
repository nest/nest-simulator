.. _dev_install:

Developer installation instructions
===================================

We encourage developes to install NEST and its dependencies in an environment, such as conda.
We have an environment.yml file that contatins all possible tools needed for NEST development.

If you require multiple versions, and different frameworks, consider using builder. Builder
is . . .

Install from source in a conda environment
------------------------------------------

* Clone nest-simulator from Github `<https://github.com/nest/nest-simulator>`_ or download a zip `<here>`_.

.. note::

  For more info on using git see our :ref:`git_workflow`

* Create a conda environment from the `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_ file.
  We recommend specifying a dedicated location ``--prefix <path/to/conda/env>`` for your environment.
  See the `conda documentation on using a custom location rather than the default envs folder <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#specifying-a-location-for-an-environment>`_

.. code-block:: sh

    conda env create -f nest-simulator/environment.yml --prefix <path/to/conda/env>
    conda activate <path/to/conda/env>

* Create a build directory:

.. code-block:: sh

    mkdir build_dir

* Change to the build directory:

.. code-block:: sh

    cd build_dir

* Configure NEST. Add the cmake option -CDMAKE_INSTALL_PREFIX:PATH=$CONDA_PREFIX to link nest to your active conda environment. ]
  You may need additional ``cmake`` options (see :ref:`cmake_options`).

.. code-block:: sh

   cmake -CDMAKE_INSTALL_PREFIX:PATH=$CONDA_PREFIX </path/to/NEST/src>

* Compile and install NEST:

.. code-block:: sh

    make
    make install
    make installcheck



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



Install from source without a virtual environment
-------------------------------------------------

The following are the basic steps to compile and install NEST from source code. See the
:ref:`CMake Options <cmake_options>` or the :ref:`High Performance Computing <hpc_install>` instructions to
further adjust settings for your system.

* If not already installed on your system, the following packages are recommended (see also the `Dependencies`_
  section)


.. note::

  The complete list of packages for an entire development environment can be found in the environment.yml file

.. code-block:: bash

    sudo apt install -y \
    libtool \
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
    ipython3 \
    python30-future \
    python3-mpi4py \
    openmpi-bin \
    libopenmpi-dev
    libmusic-dev \
    music-bin \
    python3-pip \
    python3-pytest \
    python3-pytest-timeout \
    python3-pytest-xdist

* Clone the NEST repository or unpack the tarball

.. code-block:: sh

   git clone https://github.com/<my-user>/nest-simulator/

.. code-block:: sh

    tar -xzvf nest-simulator-x.y.z.tar.gz

* Create an intall directory

   mkdir nest-x.y.z

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

NEST should now be successfully installed on your system.

* Before using NEST, make sure that all required environment variables are set correctly. In short, this can be
  established by sourcing the shell script ``nest_vars.sh``, which is installed into the path for binaries selected
  during the CMake run. See the section `Environment variables`_ for details.


Developer tools
~~~~~~~~~~~~~~~

A complete list of required packages to install NEST, run tests, and  build documentation, is available in the
``environment.yml`` file  on the `NEST simulator GitHub page <https://github.com/nest/nest-simulator>`_.

Check out all the :ref:`development workflows here <developer_space>`.


