.. _conda_tips:

Tips for installing NEST with Mamba
===================================

.. note::

   We recommend using Mamba (https://mamba.readthedocs.io).
   Mamba has the advantage of installing conda packages and
   environments more quickly and can be used as a complete drop-in replacement for conda.

This page provides a series of recommendations for installing pre-built NEST with
conda or to set up conda environments for building NEST and NEST documentation.

Basic mamba setup
-----------------

Apple systems
~~~~~~~~~~~~~


For Apple systems with an M1 chip, you must at present use `Miniforge
<https://github.com/conda-forge/miniforge>`_  and
select the ``arm64 (Apple Silicon)`` installer to create a mamba environment
that will support native builds of NEST.


Keep your base environment empty
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Your base environment should be as empty as possible in order to avoid
conflicts with other environments. Always install packages only in the new
environments (don't worry about duplicates, mamba will link the packages
if they are used in multiple environments, and not produce disk eating copies).


Get familiar with mamba environments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


To see which environments are installed on your system, use

.. code:: sh

   mamba info --envs


Installing NEST with conda-forge
--------------------------------

We provide pre-built versions of NEST on `conda-forge <https://conda-forge.org/docs/>`_.
Follow :ref:`these instructions to install NEST from conda-forge <conda_forge_install>`.


Creating a mamba environment for running and building NEST
----------------------------------------------------------

If you want to compile NEST yourself, you can create an environment containing all necessary
software for running and building NEST by executing the following command from the NEST source directory

.. code:: sh

   cd <nest-source-dir>
   mamba env create -p mamba

This will create an environment in the folder ``mamba/``. If you would like to activate the environment, use

.. code:: sh

   mamba activate mamba/

Note that the trailing slash is required for mamba not to confuse the path with a named environment (for example when
using ``--name``).


Get a good overview
-------------------

Obtain a good overview of which packages are installed where. You can use
``mamba env export -n base`` and ``mamba env export -n yournestenv``
(replacing the ``yournestenv`` name with whatever you chose). Make
sure each environment contains all dependencies.

.. note::
   Packages from your system will usually also be available in your mamba
   environment and may cause similar conflicts.
