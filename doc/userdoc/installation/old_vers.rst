.. _install_old:

For NEST 2.X
------------

You can take a look at our Conda-forge package

.. _conda_forge_install:

1. To keep your conda setup tidy, we recommend that you install NEST into
   a separate `conda environment <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html>`_
   together with Python packages that you will use when working with NEST;
   see also our :doc:`conda_tips`.

   To install the latest version of NEST in a new environment called ``ENVNAME``, just run

   .. code-block:: sh

      conda create --name ENVNAME -c conda-forge nest-simulator

   To install additional packages into the environment, just list them together with ``nest-simulator``.

   .. code-block:: sh

      conda create --name ENVNAME -c conda-forge nest-simulator jupyterlab seaborn

#. To see all NEST versions available via conda, either run

   .. code-block:: sh

      conda search -c conda-forge nest-simulator

   or browse the `conda forge file list
   <https://anaconda.org/conda-forge/nest-simulator/files>`_ (note
   there are multiple pages). To install, e.g., NEST 2.18.0, run

   .. code-block:: sh

      conda create --name nest_2_18_0 -c conda-forge nest-simulator=2.18.0=*

   The syntax for this install follows the pattern: ``nest-simulator=<version>=<build_string>``.

#. Activate your environment:

   .. code-block:: sh

      conda activate ENVNAME

#. Note the following:

   - We currently provide NEST with thread-based parallelization on Conda. This should suffice for most
     uses on personal computers.
   - Until dedicated conda builds for Apple's M1 chip (arm64) become available, you should expect relatively
     poor performance on computers with the M1 chip. You need to :doc:`build NEST yourself <mac_install>` on
     M1 systems for good performance.

NeuroFedora
-----------

The NeuroFedora team has generously provided the latest
versions of NEST on their platform. As that is available in the
standard Fedora platform repositories, it can simply be
installed using ``dnf``:

.. code-block:: bash

    sudo dnf install python3-nest

Find out more on the NeuroFedora site: https://neuro.fedoraproject.org.



