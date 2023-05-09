.. _mpi_process:

MPI process
===========


An MPI process is process of a computer program that is being executed by one or many threads and employs an implementation of the Message Passing Interface (MPI) to communicate its data with the other processes.

An example is the commonly used program `OpenMPI <https://www.open-mpi.org/>`_.

In practice, the number of MPI processes on each each node is related to the number of :ref:`threads <threads>` you want for each MPI process.
Multiplied together, we suggest that the values should equal the total number of cores in a node.

.. seealso::

    * :ref:`overview_hardware`
    * :ref:`threads`
    * :ref:`slurm_script`

After allocation of resources on which one wants to run the MPI processes, you also may want to export environment
variables related the implementation of the multiprocessing API.


.. list-table:: OpenMPI settings
   :header-rows: 1

   * - Keyword arguments
     - Description
   * - ``--enable-mpi-threads``
     - Enable thread support in OpenMPI
   * - ``--map-by socket/node``
     - map MPI processes to socket/node
   * - ``--bind-to socket/node``
     - bind MPI processes to socket/node
   * - ``--report-bindings``
     - report bindings of launched processes
   * - ``--display-allocation``
     - display detected allocation


.. seealso::

   For general details on pinning options in OpenMPI see `the HPC wiki article <https://hpc-wiki.info/hpc/Binding/Pinning>`_.
   The `Slurm documentation <https://slurm.schedmd.com/mpi_guide.html#open_mpi>`_  contains additional options for running MPI.

In addition, you can consider the number of virtual processes (Number of MPI processes x number of threads) you are using.

.. list-table::
  :header-rows: 1

  * - Number of MPI processes x threads/core
    - Description
  * - Is less than number of cores
    - Resources may be underutilized.
  * - Is equal to the number of cores
    - Resources are fully utilized (recommended for NEST)
  * - Is greater than the number of cores
    - Resources are oversubscribed

In the first two scenarios, the simulation may show better performance because it will not be slowed by processes interfering
with eachother (e.g., virtual memory thrashing).

Over-subscribing processes is typically used in development in testing and can help identify performance bottlenecks.
But performance can be degraded in this scenario.


