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



.. _environment_variables:

Environment variables
---------------------

A number of environment variables are used to specify where the components of a NEST installation are found. In
particular when installing to a custom directory, it is typically necessary to explicitly set these variables, so that
your operating system can find the NEST binaries, its libraries and custom extension modules.

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
