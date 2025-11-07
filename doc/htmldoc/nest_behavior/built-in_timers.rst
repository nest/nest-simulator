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

Detailed built-in timers can be activated (and again deactivated) prior to compilation through the CMake flag:

``-Dwith-detailed-timers=ON``.

They provide further insights into the time NEST spends in different phases of the
simulation cycle so they can be useful for :ref:`benchmarking performance <benchmark>`, but they can impact the runtime.
Therefore, detailed timers are by default inactive.

.. grid::
   :gutter: 2

   .. grid-item-card:: **Simulation run (state propagation) diagram**
     :columns: 5
     :class-item: sd-text-center

     .. graphviz:: /simulation_run.dot
        :name: Simulation run (State propagation)
        :caption: Simplified sequence of operations in the simulation run, organized in a top-down manner with a focus on timers.

     Within the `simulate timer` section, parallel processes
     (`OMP Parallel`) manage time-driven loops, handling tasks such as delivering spike data, and updating timers.
     The `OMP Master` section is responsible for gathering and communicating
     spike data, involving steps like collocating data and advancing the simulation time. `OMP
     barriers` are used to ensure thread synchronization at key points (for more details please see `Jordan et al. 2018
     <https://doi.org/10.3389/fninf.2018.00002>`_).
     The timers are indicated in white or light grey.

     For source code see: `SimulationManager::update_ <https://github.com/nest/nest-simulator/blob/5fd75c080608149b926be683d8601f28b6c32d07/nestkernel/simulation_manager.cpp#L827>`_
     and `EventDeliveryManager::gather_spike_data <https://github.com/nest/nest-simulator/blob/5fd75c080608149b926be683d8601f28b6c32d07/nestkernel/event_delivery_manager.cpp#L356>`_



   .. grid-item::
     :columns: 7

     **Multi-threaded timers**

     In previous NEST versions, only the master thread measured timers (OMP_Master).
     Since NEST 3.9, timers which measure time spent exclusively in multi-threaded environments (OMP_Parallel)
     are recorded by each thread individually.

     The legacy timer behavior can be restored via the CMake flag:

     ``-Dwith-threaded-timers=OFF``

     **Wall-time vs. CPU-time**

     All timers in NEST measure the actual wall-time spent between starting and stopping the timer. In order to only measure
     time spent on calculations, there is an additional variant for each of the timers above, suffixed with ``_cpu``. They
     can be accessed in the exact same way. For example:
     ::

         nest.time_simulate_cpu

     **MPI synchronization timer**

     In order to measure synchronization time between multiple MPI processes, an additional timer can be activated on demand
     via the CMake flag

     ``-Dwith-mpi-sync-timer=ON``.

     This timer measures the time between the end of a process' update phase
     (i.e., neuron state propagation) and start of collective communication of spikes between all MPI processes. This timer
     adds an additional MPI barrier right before the start of communication, which might affect performance.


     .. seealso::

       - For more information see the :ref:`run_simulations` guide

Kernel attribrutes for detailed timers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If detailed timers are active, the following time measurements are available as kernel attributes:

.. list-table::
   :widths: 30 40 30
   :header-rows: 1

   * - Name
     - Explanation
     - Part of
   * - ``time_gather_target_data``
     - Cumulative time for communicating connection information from postsynaptic to presynaptic side
     - ``time_communicate_prepare``
   * - ``time_communicate_target_data``
     - Cumulative time for core MPI communication when gathering target data
     - ``time_gather_target_data``
   * - ``time_update``
     - Time for neuron update
     - ``time_simulate``
   * - ``time_gather_spike_data``
     - Time for complete spike exchange after update phase
     - ``time_simulate``
   * - ``time_collocate_spike_data``
     - Time to collocate MPI send buffer from spike register
     - ``time_gather_spike_data``
   * - ``time_communicate_spike_data``
     - Time for communicating spikes between compute nodes
     - ``time_gather_spike_data``
   * - ``time_deliver_spike_data``
     - Time to deliver events from the MPI receive buffers to their local synaptic targets (including synaptic update, e.g. STDP synapses) and to the spike ring buffers of the corresponding postsynaptic neurons
     - ``time_gather_spike_data``
   * - ``time_mpi_synchronization``
     - Time spent waiting for other processes
     - ``time_communicate_spike_data``
   * - ``time_omp_synchronization_construction``
     - Synchronization time of threads during network construction.
     - ``time_construction_create``, ``time_construction_connect``, ``time_communicate_prepare``
   * - ``time_omp_synchronization_simulation``
     - Synchronization time of threads during simulation.
     - ``time_simulate``
