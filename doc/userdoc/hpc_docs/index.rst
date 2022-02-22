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

* Benchmarking

References
----------

.. [1] Kurth AC, et al. 2021. Sub-realtime simulation of a neuronal network of natural density. arXiv
       Web. https://arxiv.org/abs/2111.04398


.. todo::

   Add  benchmarking documentation here.
