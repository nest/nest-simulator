.. _built_in_timers:

Built-in timers
===============

Basic timers
------------

Basic built-in timers keep track of the time NEST spent for network construction and actual simulation (propagation of
the network state). These timers are active in all simulations with NEST, and the measured times can be checked by
querying the corresponding kernel attributes. For example:

::

    nest.time_simulate

The following basic time measurements are available:

+-------------------------------+----------------------------------+
| Name                          | Explanation                      |
+===============================+==================================+
| ``time_construction_create``  | Cumulative time NEST spent       |
|                               | creating neurons and devices     |
+-------------------------------+----------------------------------+
| ``time_construction_connect`` | Cumulative time NEST spent       |
|                               | creating connections             |
+-------------------------------+----------------------------------+
| ``time_simulate``             | Time NEST spent in the last      |
|                               | ``Simulate()``                   |
+-------------------------------+----------------------------------+

.. note::

   While preparing the actual simulation after network construction, NEST needs to build the pre-synaptic part of the
   connection infrastructure, which requires MPI communication (`Jordan et al. 2018
   <https://doi.org/10.3389/fninf.2018.00002>`__). This happens only for the first call to ``Simulate()`` unless
   connectivity changed in the meantime, and it may cause significant overhead by adding to ``time_simulate``.
   Therefore, the cumulative time NEST spent for building the pre-synaptic connection infrastructure is also tracked by
   a basic timer and available as kernel attribute ``time_communicate_prepare``.

In the context of NEST performance monitoring, other useful kernel attributes are:

+-------------------------+-----------------------------------+
| Name                    | Explanation                       |
+=========================+===================================+
| ``biological_time``     | Cumulative simulated time         |
+-------------------------+-----------------------------------+
| ``local_spike_counter`` | Number of spikes emitted by the   |
|                         | neurons represented on this MPI   |
|                         | rank during the last              |
|                         | ``Simulate()``                    |
+-------------------------+-----------------------------------+

.. note::

   ``nest.ResetKernel()`` resets all time measurements as well as ``biological_time`` and ``local_spike_counter``.


Detailed timers
---------------

Detailed built-in timers can be activated (and again deactivated) prior to compilation through the CMake flag
``-Dwith-detailed-timers=ON``. They provide further insights into the time NEST spends in different phases of the
simulation cycle, but they can impact the runtime. Therefore, detailed timers are by default inactive.

If detailed timers are active, the following time measurements are available as kernel attributes:

+-------------------------------------------+-----------------------------------+----------------------------------+
| Name                                      | Explanation                       | Part of                          |
+===========================================+===================================+==================================+
| ``time_gather_target_data``               | Cumulative time for communicating | ``time_communicate_prepare``     |
|                                           | connection information from       |                                  |
|                                           | postsynaptic to presynaptic side  |                                  |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_communicate_target_data``          | Cumulative time for core MPI      | ``time_gather_target_data``      |
|                                           | communication when gathering      |                                  |
|                                           | target data                       |                                  |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_update``                           | Time for neuron update            | ``time_simulate``                |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_gather_spike_data``                | Time for complete spike exchange  | ``time_simulate``                |
|                                           | after update phase                |                                  |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_collocate_spike_data``             | Time to collocate MPI send buffer | ``time_gather_spike_data``       |
|                                           | from spike register               |                                  |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_communicate_spike_data``           | Time for communicating spikes     | ``time_gather_spike_data``       |
|                                           | between compute nodes             |                                  |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_deliver_spike_data``               | Time to deliver events from the   | ``time_gather_spike_data``       |
|                                           | MPI receive buffers to their      |                                  |
|                                           | local synaptic targets (including |                                  |
|                                           | synaptic update, e.g. STDP        |                                  |
|                                           | synapses) and to the spike ring   |                                  |
|                                           | buffers of the corresponding      |                                  |
|                                           | postsynaptic neurons              |                                  |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_omp_synchronization_construction`` | Synchronization time of threads   | ``time_construction_create``,    |
|                                           | during network construction.      | ``time_construction_connect``,   |
|                                           |                                   | ``time_communicate_prepare``     |
+-------------------------------------------+-----------------------------------+----------------------------------+
| ``time_omp_synchronization_simulation``   | Synchronization time of threads   | ``time_simulate``                |
|                                           | during simulation.                |                                  |
+-------------------------------------------+-----------------------------------+----------------------------------+

MPI synchronization timer
-------------------------
In order to measure synchronization time between multiple MPI processes, an additional timer can be activated on demand
via the ``-Dwith-mpi-sync-timer=ON`` CMake flag. This timer measures the time between the end of a process' update phase
(i.e., neuron state propagation) and start of collective communication of spikes between all MPI processes. This timer
adds an additional MPI barrier right before the start of communication, which might affect performance.

+------------------------------+-----------------------------------------+
| Name                         | Explanation                             |
+==============================+=========================================+
| ``time_mpi_synchronization`` | Time spent waiting for other processes. |
+------------------------------+-----------------------------------------+

Multi-threaded timers
---------------------
In previous NEST versions, only the master thread measured timers. Since NEST 3.9, timers which measure time spent exclusively in multi-threaded environments are recorded by each thread individually.

The legacy timer behavior can be restored via the ``-Dwith-threaded-timers=OFF`` CMake flag.

Wall-time vs. CPU-time
-------------------------
All timers in NEST measure the actual wall-time spent between starting and stopping the timer. In order to only measure
time spent on calculations, there is an additional variant for each of the timers above, suffixed with ``_cpu``. They
can be accessed in the exact same way. For example:
::

    nest.time_simulate_cpu
