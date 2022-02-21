.. _mpi_process:

MPI process
===========


* An MPI process is a set of data and instructions with its own memory that must be explictly shared between processes.
  A process is also referred to as a task.

* We typically use the standard Message Passing Interface (MPI) to instruct how processes work in parallel (See e.g.,
  `OpenMPI <https://www.open-mpi.org/>`_).

When using more than 1 MPI process, we recommend using ``mpirun`` to run a script. This ensures each process represents
a subset of your entire script. For example, if you have 4 processes (``ntasks-per-node = 4``) and use ``mpirun -n 4``,
your script, ``my_nest_simulation.py``, will be divided up into the 4 processes. If you run ``srun`` with 4 processes, the entire simulation script
will be run 4 times.

The number of MPI processes on each each node is related to the number of threads you want for each MPI process.
Multiplied together, the values  should equal the total number of cores in a node. (The number of cores varies depending on what system you are using).

In addition, you can consider the number of physical processors (also CPUs) you are using.
See :ref:`overview_hardware`.

.. list-table:: Process to (CPU) processor scenarios
   :header-rows: 1

  * - Scenario
    - Description
  * - Less processes than processors
    - Resources may be underutilized.
  * - One process per processor
    - Resources are fully utilized
  * - More processes than processors
    - Resources are oversubscribed

In the first two scenarios, the simulation may show better performance because it will not be slowed by processes interfering
with eachother (e.g., virtual memory thrashing).

Over-subscribing processes is typically used in development in testing and can help identify performance bottlenecks.
But performance can be degraded in this scenario.



The `Slurm documentation <https://slurm.schedmd.com/mpi_guide.html#open_mpi>`_  contains additional options for running MPI.



It's usually unclear whether MPI pins threads to cores.
After allocation of resources on which one wants to run the MPI processes, one also needs to export environment
variables related the implementation of the multiprocessing API.

* OpenMPI
   * it appears that to use OpenMPI and Multithreading together one needs to, when building OpenMPI configure ``--enable-mpi-threads``
   * boost > 2.26(?) automatically contains an MPI implementation that can interfere

.. list-table:: OpenMPI settings
   :header-rows: 1

   * - Setting
     - Description
   *  - `exoort MPI_=True`
      - make it work
   *  - --map-by socket/node
      - map MPI processes to socket/node
   *  - --bind-to socket/node
      - bind MPI processes to socket/node
   *  - --rankfile rank.file
      - pass rankfile to precisely define threads on which to run a process (length of characters processed in rankfile is limited)
   *  - --report-bindings
      - report bindings of launched processes
   *  - --display-allocation
      - display detected allocation


