Ubuntu/Debian Installation
===============================

.. _standard:

Installation from source
--------------------------

The following are the basic steps to compile and install NEST from source code:


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

NEST should now be successfully installed on your system. You should now be able to ``import nest``  from a python or ipython shell.

.. admonition:: IMPORTANT!

 If your operating system does not find the ``nest`` executable or if Python does not find the ``nest`` module, your path variables may not be set correctly. This may also be the case if Python cannot load the ``nest`` module due to missing or incompatible libraries. In this case, please run:

  .. code-block:: sh

       source </path/to/nest_install_dir>/bin/nest_vars.sh

 to set the necessary environment variables. You may want to include this line in your ``.bashrc`` file, so that the environment variables are set automatically.

See the :doc:`Getting started <../getting_started>` pages to find out how to get going with NEST or check out our :doc:`example networks <../auto_examples/index>`.

Dependencies
-------------

To build NEST, you need a recent version of `CMake <https://cmake.org>`_ and `libtool <https://www.gnu.org/software/libtool/libtool.html>`_; the latter should be available for most systems and is probably already installed.

.. note:: NEST requires at least version v2.8.12 of cmake, but we recommend v3.4 or later. You can type ``cmake --version`` on the commandline to check your current version.

The `GNU readline library <http://www.gnu.org/software/readline/>`_ is recommended if you use NEST interactively **without Python**. Although most Linux distributions have GNU readline installed, you still need to install its development package if want to use GNU readline with NEST. GNU readline itself depends on `libncurses <http://www.gnu.org/software/ncurses/>`_ (or libtermcap on older systems). Again, the development packages are needed to compile NEST.

The `GNU Scientific Library <http://www.gnu.org/software/gsl/>`_ is needed by several neuron models, in particular those with conductance based synapses. If you want these models, please install the GNU Scientific Library along with its development packages.

If you want to use PyNEST, we recommend to install the following along with their development packages:

- `Python <http://www.python.org>`_
- `NumPy <http://www.scipy.org>`_
- `SciPy <http://www.scipy.org>`_
- `matplotlib <http://matplotlib.org>`_
- `IPython <http://ipython.org>`_


See the :doc:`Configuration Options <install_options>` or the :doc:`High Performance Computing <hpc_install>` instructions to  further adjust settings for your system.

What gets installed where
---------------------------

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


