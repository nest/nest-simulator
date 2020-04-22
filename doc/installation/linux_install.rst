Ubuntu/Debian Installation
==========================

.. _standard:

Dependencies
------------

To build NEST, you need a recent version of `CMake <https://cmake.org>`_ and `libtool <https://www.gnu.org/software/libtool/libtool.html>`_; the latter should be available for most systems and is probably already installed.

.. note:: NEST requires CMake 3.12 or higher, but we recommend version 3.16.X. You can type ``cmake --version`` on the commandline to check your current version.

The `GNU readline library <http://www.gnu.org/software/readline/>`_ is recommended if you use NEST interactively **without Python**. Although most Linux distributions have GNU readline installed, you still need to install its development package if want to use GNU readline with NEST. GNU readline itself depends on `libncurses <http://www.gnu.org/software/ncurses/>`_ (or libtermcap on older systems). Again, the development packages are needed to compile NEST.

The `GNU Scientific Library <http://www.gnu.org/software/gsl/>`_ is needed by several neuron models, in particular those with conductance based synapses. If you want these models, please install the GNU Scientific Library along with its development packages.

If you want to use PyNEST, we recommend to install the following along with their development packages:

- `Python 3.X <http://www.python.org>`_
- `NumPy <http://www.scipy.org>`_
- `SciPy <http://www.scipy.org>`_
- `Matplotlib 3.X <http://matplotlib.org>`_
- `IPython <http://ipython.org>`_


Installation from source
------------------------

The following are the basic steps to compile and install NEST from source code. See the :doc:`Configuration Options <install_options>` or the :doc:`High Performance Computing <hpc_install>` instructions to further adjust settings for your system.

* If not already installed on your system, the following packages are recommended (see also the `Dependencies`_ section)

.. code-block:: bash

    sudo apt-get install -y \
    cython \
    libgsl-dev \
    libltdl-dev \
    libncurses-dev \
    libreadline-dev \
    python-all-dev \
    python-numpy \
    python-scipy \
    python-matplotlib \
    python-nose \
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

* Configure NEST.

  You may need additional ``cmake`` options and you can find the :doc:`configuration options here <install_options>`

.. code-block:: sh

   cmake -DCMAKE_INSTALL_PREFIX:PATH=</install/path> </path/to/NEST/src>

.. note::
    If you want to use Python 3, add the configuration option
    ``cmake -Dwith-python=3 -DCMAKE_INSTALL_PREFIX:PATH=</install/path> </path/to/NEST/src>``

.. note::  ``/install/path`` should be an absolute path

* Compile and install NEST:

.. code-block:: sh

    make
    make install
    make installcheck

NEST should now be successfully installed on your system.

* Before using NEST, make sure that all the environment variables are set correctly. See the section `Environment variables`_ for details.

* See the :doc:`Getting started <../getting_started>` pages to find out how to get going with NEST or check out our :doc:`example networks <../auto_examples/index>`.


What gets installed where
-------------------------

By default, everything will be installed to the subdirectories ``/install/path/{bin,lib,share}``, where ``/install/path`` is the install path given to ``cmake``:

- Executables ``/install/path/bin``
- Dynamic libraries ``/install/path/lib/``
- SLI libraries ``/install/path/share/nest/sli``
- Documentation ``/install/path/share/doc/nest``
- Examples ``/install/path/share/doc/nest/examples``
- PyNEST ``/install/path/lib/pythonX.Y/site-packages/nest``
- PyNEST examples ``/install/path/share/doc/nest/examples/pynest``
- Extras ``/install/path/share/nest/extras/``

If you want to run the ``nest`` executable or use the ``nest`` Python module without providing explicit paths, you have to add the installation directory to your search paths. For example, if you are using bash:

.. code-block:: sh

    export PATH=$PATH:/install/path/bin
    export PYTHONPATH=/install/path/lib/pythonX.Y/site-packages:$PYTHONPATH

The script ``/install/path/bin/nest_vars.sh`` can be sourced in ``.bashrc`` and will set these paths for you. This also allows to switch between NEST installations in a convenient manner.


Environment variables
---------------------

There are several environment variables that describe where components of the NEST installation can be found. In particular when installing to a custom directory, it is typically necessary to explicitly set these variables, so that your operating system can find the NEST binaries and libraries.

A shell script is provided in ``</install/path/>bin/nest_vars.sh`` to make setting the environment variables more convenient. Setting the environment variables in your active shell session requires sourcing the script:

.. code-block:: sh

   source </install/path/>bin/nest_vars.sh

You may want to include this line in your ``.bashrc`` file, so that the environment variables are set automatically.

The following variables are set in ``nest_vars.sh``:

.. list-table::
   :header-rows: 1
   :widths: 10 30

   * - Path
     - Description
   * - ``NEST_INSTALL_DIR``
     - NEST installation directory. Contains ``bin``, ``lib``, etc.
   * - ``NEST_DATA_DIR``	
     - NEST finds standard *.sli files ``$NEST_DATA_DIR/sli``
   * - ``NEST_DOC_DIR``
     - NEST built-in online help finds help files ``$NEST_DOC_DIR/help``
   * - ``NEST_PYTHON_PREFIX``
     - The path where the PyNEST bindings are installed.
   * - ``PYTHONPATH``
     - Search path for non-standard Python module locations. Will be prepended to or created if it does not exist.
   * - ``PATH``
     - Search path for binaries. Will be prepended to or created if it does not exist.
   * - ``LD_LIBRARY_PATH``
     - Search path for shared objects (*.so files). Note: called ``DYLD_LIBRARY_PATH`` on MacOS.  Will be prepended to or created if it does not exist.

If your operating system does not find the ``nest`` executable or if Python does not find the ``nest`` module, your path variables may not be set correctly. This may also be the case if Python cannot load the ``nest`` module due to missing or incompatible libraries.
