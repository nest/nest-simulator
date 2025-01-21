.. _conda_forge_install:

conda-forge install
===================

.. admonition:: osx-arm64: missing random number generators

   Due to a cross-compiling issue in the conda-forge NEST package, some random number
   generators are not available if you are using macOS arm64 architecture.
   The available generators are the Mersenne Twister generators `mt19937` and `mt19937_64`.

.. note::

   We recommend using Mamba (https://mamba.readthedocs.io).
   Mamba has the advantage of installing conda packages and
   environments more quickly and can be used as a complete drop-in replacement for conda.

1. To keep your mamba setup tidy, we recommend that you install NEST into
   a separate environment
   together with Python packages that you will use when working with NEST;
   see also our :ref:`conda_tips`.

   To install the latest version of NEST in a new environment called ``ENVNAME``, just run

   .. code-block:: sh

      mamba create --name ENVNAME -c conda-forge nest-simulator

   To install additional packages into the environment, just list them together with ``nest-simulator``.

   .. code-block:: sh

      mamba create --name ENVNAME -c conda-forge nest-simulator jupyterlab seaborn

#. To see all NEST versions available via conda, either run

   .. code-block:: sh

      mamba search -c conda-forge nest-simulator

    To install a specific version  e.g., NEST 2.18.0, run

   .. code-block:: sh

      mamba create --name nest_2_18_0 -c conda-forge nest-simulator=2.18.0=*

   The syntax for this install follows the pattern: ``nest-simulator=<version>=<build_string>``.

#. Activate your environment:

   .. code-block:: sh

      mamba activate ENVNAME

#. Note the following:

   - We currently provide NEST with thread-based parallelization on conda-forge. This should suffice for most
     uses on personal computers.
