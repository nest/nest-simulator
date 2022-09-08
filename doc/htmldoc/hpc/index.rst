.. _hpc_index:

All about high performance computing
====================================

Optimize NEST performace
------------------------


For large simulations, optimizing your system will make running NEST faster and more efficient.

One way to optimize performance is to minimize interference between computations being performed on multicore and
multi processor systems. This is done by selecting the best parameters for the hardware you're working with.


See also Anno et al. [1]_ for detailed study on this topic.

.. toctree::
   :hidden:

   overview_hardware
   slurm_script
   threading
   mpi_processes
   benchmarking
   parallel_computing

If you are unfamiliar with hardware and software components in HPC systems start here:


* :ref:`Overview of hardware and software terminology <overview_hardware>`


We have an example job script using standard configuration options.

* :ref:`Example job script (Slurm) <slurm_script>`

For details on software allocation components, see

* :ref:`Threading <threads>`

* :ref:`MPI processes <mpi_process>`


Understand parallelism in NEST
------------------------------

To understand how NEST utilizes thread parallel and distributed computing in simulations, see our :ref:`parallel_computing`.
We explain how neurons, devices, and synapses in NEST intersect with threads and processes in parallel setups.

.. admonition:: Speed up parallel simulations

    During network construction, create all nodes of one type (e.g., neurons) followed by all nodes of another type (e.g., devices).
    See :py:func:`.Create`.
    For comparison tests, see `this GitHub thread <https://github.com/nest/nest-simulator/pull/2290>`_.



Benchmarking NEST
-----------------

* We recommend :ref:`the benchmarking framework beNNch <benchmark>` developed by Albers et al [2]_.

* Example PyNEST script: :doc:`../auto_examples/hpc_benchmark`

References
----------

.. [1] Kurth AC. Senk J. Terhorst D. Finnerty J. and Diesmann M (2022). Sub-realtime simulation of a neuronal network of natural density.
       Neuromorphic Computing and Engineering(2):021001. https://doi.org/10.1088/2634-4386/ac55fc

.. [2] Albers J., et al (2022). A Modular Workflow for Performance Benchmarking of Neuronal Network Simulations.
       Frontiers in Neuroinformatics(16):837549. https://doi.org/10.3389/fninf.2022.837549
