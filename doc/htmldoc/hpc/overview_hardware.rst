.. _overview_hardware:

Overview of various hardware and software components
====================================================

To optimize NEST performance, it's important to understand the system you are using and its components.
Here we try to provide a brief description of the generic setup of hardware and corresponding software.
Note that these are not terms used specifically for NEST, but are common in HPC organizations.

.. note::

  This is just one configuration for hardware setup. A particular system may use other components, but for our needs
  nodes and cores are the most important terms to know.

  Also note, a single component may be called something different by different
  organizations, manufacturers, and companies. We also have overlapping terms, such as nodes, that have one meaning
  in computational neuroscience, and another in computer hardware. Here we try to use the most common terms, but
  always check the terminology used in your system.



.. seealso::

    * :ref:`slurm_script`
    * :ref:`threads`
    * :ref:`mpi_process`


.. image:: ../static/img/hpc_ware.png
    :align: center
    :scale: 80%

.. grid:: 1 1 2 2

    .. grid-item-card:: Physical components (left side of image)

        This a representation of a typical hardware setup.

        A supercomputer or cluster will have many nodes.

        A node can contain sockets where the individual processors (also called CPUs) are located.
        Each processor has some number of cores, which execute computations and cache (L1, L2, L3) as local memory store.


    .. grid-item-card:: Software components (right side of image)

        This is how the data and instructions are allocated through software.

        * A set of data and instructions that belong together is referred to as a task or process. This can be your entire simulation
          script or a subset of it.
          To allow processes to run in parallel, we typically use the standard Message Passing Interface (MPI) 
          to instruct how they work (See e.g., `OpenMPI <https://www.open-mpi.org/>`_).

        * The smallest unit of executable program is known as a thread. We can control threads following standards like`OpenMP <https://www.openmp.org/>`_.

To efficiently run your large and complex simulation, you need to configure the optimal number of :ref:`threads <threads>` and :ref:`processes <mpi_process>` for
your simulation and the given hardware of the HPC system you are using.


