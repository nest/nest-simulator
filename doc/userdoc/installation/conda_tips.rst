Tips for installing NEST with conda
===================================

This page provides a series of recommendations for installing pre-built NEST with
conda or to set up conda environments for building NEST and NEST documentation.


Basic conda setup
-----------------

Choice of conda base installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We test NEST in conda environments using Miniconda installations and thus recommend
that you do the same. The recommendations that we provide here will also likely work with a
full-sized Anaconda installation, but we can only provide limited support for this.

You can either install

- Miniconda from `<https://docs.conda.io/en/latest/miniconda.html>`_
- Miniforge from `<https://github.com/conda-forge/miniforge>`_

For Apple systems with an M1 chip, you must at present use Miniforge and 
select the ``arm64 (Apple Silicon)`` installer to create a conda environment
that will support native builds of NEST.


Keep your base environment empty
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Your base environment should be as empty as possible in order to avoid
conflicts with other environments. Always install packages only in the new
environments (don't worry about duplicates, conda will link the packages
if they are used in multiple environments, and not produce disk eating copies).


Get familiar with conda environments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Conda environments are a powerful tool. See the `Conda documentation on Managing Environments 
<https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html>`_
for more information.

To see which environments are installed on your system, use

.. code:: sh

   conda info --envs
   

Installing NEST with Conda
--------------------------

We provide pre-built versions of NEST on `Conda Forge <https://anaconda.org/conda-forge/nest-simulator/files>`.
Follow :ref:`these instructions to install NEST from Conda Forge <conda_forge_install>`.


Creating a Conda environment for running and building NEST
----------------------------------------------------------

If you want to compile NEST yourself, you can create an environment containing all necessary 
software for running and building NEST by executing the following command from the NEST source directory

.. code:: sh

   conda env create -f extras/conda-nest-simulator-dev.yml
   
This will create an environment called ``nest-simulator``. If you would like to give the environment
a different name, use

.. code:: sh

   conda env create -f extras/conda-nest-simulator-dev.yml -n MYNAME
   
   
Get a good overview
-------------------

Obtain a good overview of which packages are installed where. You can use
``conda env export -n base`` and ``conda env export -n yournestenv``
(replacing the ``yournestenv`` name with whatever you chose). Make
sure each environment contains all dependencies. One way to make
this obvious would be to reduce conda stack to ``0`` (conda documentation
`here <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#nested-activation>`_),
and/or to a certain degree by not auto-activating the base environment (conda documentation
`here <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#conda-init>`_).
Then packages from base do not 'leak' into your new environments.

.. note::
   Packages from your system will usually also be available in your conda
   environment and may cause similar conflicts.
