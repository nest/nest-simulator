.. _linux_install:

Ubuntu/Debian Installation
==========================

.. _standard:

Dependencies
------------

To build NEST, you need a recent version of `CMake <https://cmake.org/install>`_ and
`libtool <https://www.gnu.org/software/libtool/libtool.html>`_; the latter should be available for most systems and is
probably already installed.

.. note::

   NEST requires CMake 3.12 or higher, but we recommend version 3.16. You can type ``cmake --version`` on the
   commandline to check your current version.

The `GNU readline library <http://www.gnu.org/software/readline/>`_ is recommended if you use NEST interactively
**without Python**. Although most Linux distributions have GNU readline installed, you still need to install its
development package if you want to use GNU readline with NEST. GNU readline itself depends on
`libncurses <http://www.gnu.org/software/ncurses/>`_ (or libtermcap on older systems). Again, the development packages
are needed to compile NEST.

The `GNU Scientific Library <http://www.gnu.org/software/gsl/>`_ is needed by several neuron models, in particular
those with conductance based synapses. If you want these models, please install the GNU Scientific Library along with
its development packages.

For efficient sorting algorithms the `Boost library <https://www.boost.org/>`_ is used. Since this is an essential
factor for the communication of spikes, some simulations are significantly faster when NEST is compiled with Boost.

If you want to use PyNEST, we recommend to install the following along with their development packages:

- `Python 3.8 or higher <http://www.python.org>`_
- `Cython 0.28.3 or higher <https://cython.org>`_
- `NumPy <http://www.numpy.org>`_
- `SciPy <http://www.scipy.org>`_
- `Matplotlib 3.0 or higher <http://matplotlib.org>`_
- `IPython <http://ipython.org>`_


.. _source-install:

Installation from source in a virtual Python environment
--------------------------------------------------------

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

* Unpack the tarball

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

* See the :ref:`Getting started <getting_started>` pages to find out how to get going with NEST or check out our
  :ref:`example networks <pynest_examples>`.


Installation from source without a virtual Python environment
-------------------------------------------------------------

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
    python3-all-dev \
    python3-numpy \
    python3-scipy \
    python3-matplotlib \
    python3-nose \
    python3-junitparser \
    python3-mpi4py \
    openmpi-bin \
    libopenmpi-dev

* Unpack the tarball

.. code-block:: sh

    tar -xzvf nest-simulator-x.y.z.tar.gz

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

   cmake -DCMAKE_INSTALL_PREFIX:PATH=<nest_install_dir> </path/to/NEST/src>

.. note::

   ``/install/path`` should be an absolute path

.. note::

   Python bindings are enabled by default. Add the configuration option ``-Dwith-python=OFF`` to disable them.

* Compile and install NEST:

.. code-block:: sh

    make
    make install
    make installcheck

NEST should now be successfully installed on your system.

* Before using NEST, make sure that all required environment variables are set correctly. In short, this can be
  established by sourcing the shell script ``nest_vars.sh``, which is installed into the path for binaries selected
  during the CMake run. See the section `Environment variables`_ for details.

* See the :ref:`Getting started <getting_started>` pages to find out how to get going with NEST or check out our
  :ref:`example networks <pynest_examples>`.


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


.. _environment_variables:

Environment variables
---------------------

A number of environment variables are used to specify where the components of a NEST installation are found. In
particular when installing to a custom directory, it is typically necessary to explicitly set these variables, so that
your operating system can find the NEST binaries, its libraries and custom extension modules.

For your convenience, a shell script setting all required environment variables is provided in
``<nest_install_dir>/bin/nest_vars.sh``. Setting the environment variables in your active shell session requires
sourcing the script:

.. code-block:: sh

   source <nest_install_dir>/bin/nest_vars.sh

You may want to include this line in your ``.bashrc`` file, so that the environment variables are set automatically
whenever you open a new terminal.

The following variables are set in ``nest_vars.sh``:

.. list-table::
   :header-rows: 1
   :widths: 10 30

   * - Variable
     - Description
   * - ``PYTHONPATH``
     - Search path for non-standard Python module locations. Will be newly set or prepended to the already existing
       variable if it is already set.
   * - ``PATH``
     - Search path for binaries. Will be newly set or prepended to the already existing variable if it is already set.

If your operating system does not find the ``nest`` executable or if Python does not find the ``nest`` module, your
path variables may not be set correctly. This may also be the case if Python cannot load the ``nest`` module due to
missing or incompatible libraries.
