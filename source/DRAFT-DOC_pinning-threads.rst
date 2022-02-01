.. _advanced_hpc:

Additional configuation options
-------------------------------

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


Threading
~~~~~~~~~


.. list-table:: OpenMP settings
   :header-rows: 0

   * - export OMP_NUM_THREADS=#CPUSPERTASK#
     - variable telling OpenMP how many threads are used on a MPI process
   * - export OMP_PROC_BIND=true
     - no movement of threads between OpenMP threads and OpenMP places
   * - export OMP_PROC_BIND=close
     - no movement of threads between OpenMP threads and OpenMP places and OpenMP places are 'close' in a hardware sense
   * - export OMP_PLACES=threads/cores
     - each OpenMP place corresponds to a hardware thread/core
   * - export OPM_PLACES="{a : b : c}"
     - OpenMP places are a, a+b, a+2c, ... a+nc=b (numbering usually relates to cores/hardware threads)
   * - export OPM_DISPLAY_ENV=true
     - display OpenMP variables


.. _pinning_threads:

Pinning threads
~~~~~~~~~~~~~~~

Pinning threads allows you to control the distribution of threads across available cores on your system, and is particularly
useful in high performance computing (HPC) systems.



Sequential pinning scheme
`````````````````````````

Description of what you're looking at

.. figure:: _static/img/CPUs-lin-lin.gif

   sequential placing - cores closely packed

   fill cores of 1 processor before going to next

- possible disadvantage
   - threads need to fight for resources - slower
- possible advantages
   - could shut off 2nd processor - save energy
   - could share memory prefetching

Distant pinning scheme
``````````````````````

.. figure:: _static/img/CPUs-lin-sparse.gif

   distant placing

   maximizes distance between threads in hardware



What are the commands here to set sequential / distant?

Helpful commands
----------------

Here are some commands that can help you understand your system better.
You can run them locally on any Linux machine as well as on HPC systems.

* ``lstopo``
     lstopo and lstopo-no-graphics are capable of displaying a topological map of the system

* ``htop``
      It is similar to top, but allows you to scroll vertically and horizontally, so you can see all the processes
      running on the system, along with their full command lines.



References
----------

.. [1] Kurth AC, et al. 2021. Sub-realtime simulation of a neuronal network of natural density. arXiv
       Web. https://arxiv.org/abs/2111.04398



