.. _threads:

Threading
=========

The smallest unit of executable program is known as a thread and is a virtual component.

We can adjust the number of :ref:`MPI processes <mpi_process>` and threads, but the product of the two  must be the total number of cores.
For example, with 8 cores, we can have 2 processes and 4 threads per process or 1 process with 8 threads.
In NEST, we recommend only having one thread per core.

We can control the number and placement of threads with programs like `OpenMP <https://www.openmp.org/>`_.

.. seealso::

    * :ref:`overview_hardware`
    * :ref:`mpi_process`
    * :ref:`slurm_script`

.. _pinning_threads:

Pinning threads
--------------

Pinning threads allows you to control the distribution of threads across available cores on your system, and is particularly
useful in high performance computing (HPC) systems.

See also Anno et al. 2022 [1]_ for detailed investigation into this topic.

We recommend that you include  ``OMP_NUM_THREADS=$CPUS_PER_TASK`` and ``OMP_PROC_BIND=True`` in your :ref:`job script <slurm_script>`.

For more advanced cases, you may want to look into pinning schemes, detailed below.

.. list-table:: OpenMP settings
   :header-rows: 1

   * - Setting
     - Description
   * - ``export OMP_NUM_THREADS=#CPUSPERTASK#``
     - variable telling OpenMP how many threads are used on a MPI process
   * - ``export OMP_PROC_BIND=true``
     - no movement of threads between OpenMP threads and OpenMP places
   * - ``export OMP_PROC_BIND=close``
     - no movement of threads between OpenMP threads and OpenMP places and OpenMP places are 'close' in a hardware sense
   * - ``export OMP_PLACES=threads/cores``
     - each OpenMP place corresponds to a hardware thread/core
   * - ``export OPM_PLACES="{a : b : c}"``
     - OpenMP places are a, a+b, a+2c, ... a+nc=b (numbering usually relates to cores/hardware threads)
   * - ``export OPM_DISPLAY_ENV=true``
     - display OpenMP variables

.. seealso::

   For general details on pinning in HPC systems see `the HPC wiki article <https://hpc-wiki.info/hpc/Binding/Pinning>`_.


Sequential pinning scheme
`````````````````````````

.. figure:: ../static/img/CPUs-lin-lin.gif

   Sequential placing

   In this scheme, the cores of 1 processor are filled before going to next

   Setting to use for this case: ``export OMP_PROC_BIND = close``

- Possible disadvantage
   - This scheme could be slower: threads need to fight for resources
- Possible advantages
   - You might save enrergy: shut off unused processors
   - This scheme may allow sharing of memory prefetching

Distant pinning scheme
``````````````````````

.. figure:: ../static/img/CPUs-lin-sparse.gif

   Distant placing

   Maximizes distance between threads in hardware

   Setting to use for this case: ``export OMP_PROC_BIND = spread``

- Possible disadvantage
   - This scheme could increase energy usage, as multiple processors are used
- Possible advantage
   - It could be faster: resources available for each thread


References
----------

.. [1] Kurth AC. Senk J. Terhorst D. Finnerty J. and Diesmann M (2022). Sub-realtime simulation of a neuronal network of natural density.
       Neuromorphic Computing and Engineering(2):021001. https://doi.org/10.1088/2634-4386/ac55fc




