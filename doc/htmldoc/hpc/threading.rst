.. _threads:

Threading
=========


* The smallest unit of executable program is known as a thread. The thread is a virtual component. A single core can
  have one or two threads. Therefore the total number of possible threads is double the number of cores. In NEST, we recommend
  only having one thread per core.
  See :ref:`overview_hardware`.


* You can adjust the number of threads in relation to the number of :ref:`MPI processes <mpi_process>` you use.



.. _pinning_threads:

Pinning threads
--------------

Pinning threads allows you to control the distribution of threads across available cores on your system, and is particularly
useful in high performance computing (HPC) systems.

See also Anno et al. [1]_.

We recommend that you include  ``OMP_NUM_THREADS=$CPUS_PER_TASK`` and ``OMP_PROC_BIND=True`` in your :ref:`job script <slurm_script>`.

For more advanced cases, you may want to look into pinning schemes, detailed below.

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




Sequential pinning scheme
`````````````````````````

Description of what you're looking at

.. figure:: ../static/img/CPUs-lin-lin.gif

   sequential placing - cores closely packed

   fill cores of 1 processor before going to next

- possible disadvantage
   - threads need to fight for resources - slower
- possible advantages
   - could shut off 2nd processor - save energy
   - could share memory prefetching

Distant pinning scheme
``````````````````````

.. figure:: ../static/img/CPUs-lin-sparse.gif

   distant placing

   maximizes distance between threads in hardware





References
----------

.. [1] Kurth AC, et al. 2021. Sub-realtime simulation of a neuronal network of natural density. arXiv
       Web. https://arxiv.org/abs/2111.04398



