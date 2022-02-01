HPC docs
========


Optimize NEST performace on high performance computers (HPC)
------------------------------------------------------------


For large simulations, optimizing your system will make running NEST faster and more efficient.

One way to optimize performance is to minimize interference between computations being performed on multicore and
multi processor systems. This is done by selecting the best parameters for the hardware you're working with.


See also Anno et al. [1]_ for detailed study on this topic.

.. toctree::
   :hidden:

   overview_hardware
   slurm_script
   pinning-threads
   advanced_config

* :ref:`Overview of hardware and software terminology <overview_hardware>`

* :ref:`Example job script (standard configuration options for HPC systems) <slurm_script>`

* :ref:`Advanced settings: Pinning threads <pinning_threads>`

* :ref:`Additional settings (move to benchmarking?) <advanced_hpc>`

Understanding parallelism in NEST
---------------------------------

To understand how NEST utilizes thread parallel and distributed computing in simulations, see our guide on
:ref:`parallel_computing`.
We explain how neurons, devices, and synapses in NEST intersect with threads and processes in parallel setups.


Benchmarking NEST
-----------------

* Benchmarking

References
----------

.. [1] Kurth AC, et al. 2021. Sub-realtime simulation of a neuronal network of natural density. arXiv
       Web. https://arxiv.org/abs/2111.04398



