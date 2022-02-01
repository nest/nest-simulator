.. _overview_hardware:

Overview of various hardware and software components
====================================================


.. image:: _static/img/hpc_ware.png




.. note::

  This is just one configuration for hardware setup. A particular system may use other components, but for our needs
  nodes and cores are the most important terms to know.

In the image above, on the left side we have a representation of a typical hardware setup.

* In a supercomputer or cluster, there are many, many nodes.

* Each node contains CPUs (or processors) and each CPU has its own cores and cache (L1, L2, L3).

* Each node is basically its own computer and comprises other components not shown here.

* The cores are where the computations are performed.

* The cache is the local memory store for that processor.

* Data and instructions (your code) are retrieved from the RAM (not shown) and passed to the cache and allocated to the cores.

* The cores execute the instructions and return output to the cache.

On the right side, we see how the data and instructions are allocated through software.

* A set of data and instructions that belong together is referred to as a task or process. This can be your entire simulation
  script or a subset of it.

* We typically use the standard Message Passing Interface (MPI) to instruct how processes work in parallel (See e.g.,
  `OpenMPI <https://www.open-mpi.org/>`_).

* We can adjust the number of processes and threads. For example, instead of the 2 processes and 4 threads per process in the
  image above, we could have 1 process with 8 threads.

* The smallest unit of executable program is known as a thread. The thread is a virtual component. A single core can
  have one or two threads. Therefore the total number of possible threads is double the number of cores. In NEST, we recommend
  only having one thread per core.

* We can control the number and placement of threads with programs like `OpenMP <https://www.openmp.org/>`_.


To efficiently run your large and complex simulation, you need to configure the optimal number of threads and processes for
your simulation and the given hardware of the HPC system you are using.

The section :ref:`slurm_script`  provides a detailed example of setting up a job script with the
correct configuratioin options.



