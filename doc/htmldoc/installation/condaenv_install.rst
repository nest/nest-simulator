.. _condaenv:

Install from source in a mamba environment
==========================================

.. note::

   We recommend using Mamba (https://mamba.readthedocs.io).
   Mamba has the advantage of installing conda packages and
   environments more quickly and can be used as a complete drop-in replacement for conda.

* Create a mamba environment from the `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_ file.
  We recommend specifying a dedicated location (``-p <path/to/mamba/env>``) for your environment, for example, a new directory ``nest-mamba-env`` in your home directory or where you cloned the NEST repository.

.. code-block:: sh

    mamba env create -f nest-simulator/environment.yml -p <path/to/mamba/env>
    mamba activate <path/to/mamba/env>

* Create a build directory:

.. code-block:: sh

    mkdir build_dir

* Change to the build directory:

.. code-block:: sh

    cd build_dir

* Configure the NEST build using the following CMake command. We add the CMake option ``-CDMAKE_INSTALL_PREFIX:PATH=$CONDA_PREFIX`` to link NEST to the active mamba environment.
  You may need additional ``cmake`` options (see :ref:`cmake_options`).

.. code-block:: sh

   cmake -DCMAKE_INSTALL_PREFIX:PATH=$CONDA_PREFIX </path/to/NEST/src>

* Compile and install NEST:

.. code-block:: sh

    make
    make install
    make installcheck

.. note::

   To build the developer or user documentation see :ref:`doc_workflow`
