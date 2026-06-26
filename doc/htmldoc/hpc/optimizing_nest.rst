.. _optimize_performance:

Optimize performance of HPC Systems
===================================


If you are new to running NEST on HPC systems or trying to improve the performance or debug issues,
we have provided a few guides to help you out. There are a few things to consider about the
hardware and software of the HPC system you are using for improving the overall
performance.


Although there will be some variation between scripts and HPC systems, in general, we recommend that

 * resources are fully utilized (e.g., all available cores are used).
 * one thread pinned to one core (no simultaneous multithreading)
 * More than one MPI process is used (for NEST 3)

.. grid:: 1 1 2 2

    .. grid-item-card:: Overview of HPC systems
         :class-title: sd-d-flex-row sd-align-minor-center
         :link: overview_hardware
         :link-type: ref

         Get an overview of common hardware and software components in HPC systems

    .. grid-item-card:: Example SLURM script
         :class-title: sd-d-flex-row sd-align-minor-center
         :link: slurm_script
         :link-type: ref

         See a typical job script, with detailed descriptions of each line.

.. grid:: 1 1 2 2

    .. grid-item-card:: Threading
         :class-title: sd-d-flex-row sd-align-minor-center
         :link: threads
         :link-type: ref

         Learn about threading and useful OpenMP settings

    .. grid-item-card:: MPI processes
         :class-title: sd-d-flex-row sd-align-minor-center
         :link: mpi_process
         :link-type: ref

         Get some tips on MPI processes and OpenMPI


.. seealso::

   See our other guides on

   * :ref:`parallel_computing`
   * :ref:`benchmark`

   or see our performance results:

   * :doc:`NEST performance benchmarks <benchmarks:index>`
