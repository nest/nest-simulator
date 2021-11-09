Pinning threads
===============

Pinning threads allows you to control the distribution of threads across available cores on high performance
computing (HPC) system.
This can improve the performance of your code.

Here you can find some general configuration options as well as some system specific options that we recommend.

Configuration options
---------------------

Try setting settings at the hightest level first.

Scheduler
~~~~~~~~~

* Slurm

.. list-table:: General Slurm settings
   :header-rows: 1

   * - Setting
     - Description
   * - `export CPU_AFFINITY=True`
     - make stuff do something
   * - `--exclusive`
     - Always set this toa void other processes on the same node

.. list-table:: Slurm settings on specific machines
   :header-rows: 1

   * - Machine
     - Setting
     - Description
   * - JURECA
     -
     -
   * - JUSUF
     -
     -
   * - Piz Daint
     -
     -

* PBS / Torque

.. list-table:: PBS / Torque settings on specific machines
   :header-rows: 1

   * - Machine
     - Setting
     - Description
MPI
~~~
It's usually not clear whether MPI pins threads to cores.
After allocation of ressources on which one wants to run the MPI processes, one also needs to export environemnt variables related the implementation of the multiprocessing API.  

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

* ParastationMPI

Threading
~~~~~~~~~

 * hardware threading settings
 * power capping

* OpenMP

.. list-table:: OpenMP settings
   :header-rows: 1

   * - Setting
     - Description
   * - `--map-by-core`
     - make it work
   * - `--report-bindings`
     -
   * - `OMP_PLACES`
     -
   * - `OMP_PROC_BIND`
     -
   * - `JAFFINITY=TRUE`
     -
   * - `--bind-to core:overload-allowed`
     -


