.. _advanced_hpc:

Advanced configuation options
=============================

* Additional ways to optimize performance
* Useful for benchmarking


These configuration options may help improve performance, but are not necessary to include in your script.
If you are performing benchmarks or if you think your simulation can perform better, take a look at the following options.


.. list-table:: Additional Slurm settings
   :header-rows: 1

   * - Setting
     - Description
   * - `export CPU_AFFINITY=True`
     - make stuff do something
   * - `--exclusive`
     - Always set this to avoid other processes on the same node


MPI
~~~

It's usually unclear whether MPI pins threads to cores.
After allocation of resources on which one wants to run the MPI processes, one also needs to export environemnt
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

Helpful commands
----------------

Here are some commands that can help you understand your system better.
You can run them locally on any Linux machine as well as on HPC systems.

* ``lstopo``
     lstopo and lstopo-no-graphics are capable of displaying a topological map of the system

* ``htop``
      It is similar to top, but allows you to scroll vertically and horizontally, so you can see all the processes
      running on the system, along with their full command lines.


