.. _condaenv:

Install from source in a conda environment
==========================================

* Create a conda environment from the `environment.yml <https://github.com/nest/nest-simulator/blob/master/environment.yml>`_ file.
  We recommend specifying a dedicated location (``-p <path/to/conda/env>``) for your environment.
  See the `conda documentation <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#specifying-a-location-for-an-environment>`_
  on using a custom location rather than the default envs folder.

.. code-block:: sh

    conda env create -f nest-simulator/environment.yml --p <path/to/conda/env>
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

   cmake -DCMAKE_INSTALL_PREFIX:PATH=$CONDA_PREFIX </path/to/NEST/src>

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



.. note::

   To build the developer or user documentation see :ref:`doc_workflow`


