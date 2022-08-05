:orphan:

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

* :ref:`Overview of hardware and software terminology <overview_hardware>`

* :ref:`Example job script (standard configuration options for HPC systems) <slurm_script>`

* :ref:`Threading <threads>`

* :ref:`MPI processes <mpi_process>`


Understand parallelism in NEST
------------------------------

To understand how NEST utilizes thread parallel and distributed computing in simulations, see our guide on
:ref:`parallel_computing`.
We explain how neurons, devices, and synapses in NEST intersect with threads and processes in parallel setups.

Speed up simulations
--------------------

* #2290 - Speed up look up of nodes

Benchmarking NEST
-----------------

* See the benchmarking framework beNNch developed by Albers et al [2]_.

* Example :doc:`../auto_examples/hpc_benchmark`

References
----------

.. [1] Kurth AC. Senk J. Terhorst D. Finnerty J. and Diesmann M (2022). Sub-realtime simulation of a neuronal network of natural density. 
       Neuromorphic Computing and Engineering(2):021001. https://doi.org/10.1088/2634-4386/ac55fc

.. [2] Albers J., et al (2022). A Modular Workflow for Performance Benchmarking of Neuronal Network Simulations. 
       Frontiers in Neuroinformatics(16):837549. https://doi.org/10.3389/fninf.2022.837549
