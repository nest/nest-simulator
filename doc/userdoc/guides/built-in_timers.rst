Built-in timers
===============

Basic timers
------------

Basic built-in timers keep track of the time NEST spent for network construction and actual simulation (propagation of
the network state). These timers are active in all simulations with NEST, and the measured times can be queried using
``GetKernelStatus``. For example:

::

    nest.GetKernelStatus('time_simulate')

The following basic time measurements are available:

====================================  =====================================
Name                                  Explanation
====================================  =====================================
``time_construction_create``          Cumulative time NEST spent creating
				      neurons and devices
``time_construction_connect``         Cumulative time NEST spent creating
                                      connections
``time_simulate``                     Time NEST spent in the last
                                      ``Simulate()``
====================================  =====================================

.. note ::

   After network construction in preparation of the actual simulation, NEST needs to build the pre-synaptic part of the connection infrastructure, which requires MPI communication (`Jordan et al. 2018 <https://doi.org/10.3389/fninf.2018.00002>`__). This happens only for the first call to ``Simulate()`` unless connectivity changed meanwhile, and it may cause significant overhead adding to ``time_simulate``. Therefore, the cumulative time NEST spent for building the pre-synaptic connection infrastructure is also tracked by a basic timer and available in the kernel dictionary as ``time_communicate_prepare``.

In the context of NEST performance monitoring, other useful kernel-dictionary items are:

====================================  =====================================
Name                                  Explanation
====================================  =====================================
``biological_time``                   Cumulative simulated time 
``local_spike_counter``               Number of spikes emitted by the
                                      neurons represented on this MPI rank
				      during the last ``Simulate()``
====================================  =====================================

.. note ::

   ``nest.ResetKernel()`` resets all time measurements as well as ``biological_time`` and ``local_spike_counter``.

		


