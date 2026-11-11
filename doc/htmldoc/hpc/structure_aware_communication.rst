.. _structure_aware_communication:

Structure-aware communication (morph)
=====================================

Many large network models consist of areas: neurons connect densely and with short delays within an area, and
sparsely and with longer delays between areas. Structure-aware communication, also called *morph*, uses this
structure to reduce the cost of spike communication between MPI processes [1]_.

.. note::

   Structure-aware communication is available only if NEST was built with ``-Dwith-morph=ON``, which requires
   ``-Dwith-mpi=ON`` (see :ref:`cmake_options`). ``nest.build_info["have_morph"]`` tells whether the installed NEST
   contains it. A build without the option contains no code of morph and behaves as before.

How it works
------------

NEST exchanges spikes between MPI processes once every ``min_delay``, the smallest delay in the network (see
:ref:`parallel_computing`). Each exchange is a collective communication among all processes, and its cost grows
with the number of processes.

Morph splits the connections into two paths:

* The **short path** carries the connections within an area. Spikes of these connections are exchanged every
  ``min_delay``, but only among the processes of a *local group*, which hosts one or a few areas.
* The **long path** carries the connections created with the ``conn_spec`` key ``long_range``, typically between
  areas. Their delay must be at least ``threshold_delay``. Spikes of these connections are collected over
  ``threshold_delay / min_delay`` cycles and exchanged among all processes once per interval of ``threshold_delay``.

Spikes keep their time stamps on both paths, so a simulation with morph produces the same spikes as a simulation
with the conventional exchange and the same placement of neurons on processes.

When it helps
-------------

Morph reduces the number of global exchanges by the factor ``threshold_delay / min_delay``. It helps if

* the network divides into areas such that connections between areas have delays of at least ``threshold_delay``
  and connections within areas have short delays, and
* the work per process is balanced: with one area per process, every process waits at the global exchange for the
  process with the most work.

If the activity differs strongly between areas, the process of the most active area sets the pace of every
global exchange, and morph can be slower than the conventional exchange. In such cases, a placement that balances
the load matters more than the communication scheme.

Kernel parameters
-----------------

``threshold_delay`` (ms)
    Threshold delay of the long path. The default, infinity, switches morph off. The value must be positive and a
    multiple of ``min_delay`` (checked at :py:func:`.Simulate`), and it must not exceed 16383 simulation steps.
    Set it before creating any connection; it cannot change after the simulation has started.

``comm_local_ranks`` (list of int)
    Ranks of the local group of the calling process, including its own rank. The default is the own rank only:
    every process forms a group of its own. The groups must partition the processes. Each process passes the
    ranks of its own group, and **all processes must set the parameter in the same call** to
    :py:func:`.SetKernelStatus`, because NEST checks the groups collectively; if some processes do not take part,
    the simulation deadlocks. Set the parameter before creating any connection. :py:func:`.ResetKernel` restores
    the default.

``buffer_size_spike_data_long_delay`` (int)
    Initial size of the MPI buffer of the long path, at least twice the number of MPI processes. The buffer adapts
    during the simulation, as the buffer of the conventional exchange does.

Connections
-----------

Connections enter the long path with the ``conn_spec`` key ``long_range``:

.. code-block:: python

    nest.Connect(source_area, target_area, {"rule": "fixed_indegree", "indegree": 10, "long_range": True},
                 {"delay": nest.random.uniform(1.0, 3.0)})

* The delay of a long-range connection must be at least ``threshold_delay``, also when it is changed with ``set``
  on a SynapseCollection.
* Only connections between neurons that transmit spikes can be long-range; connections from or to devices and
  connections of synapse models for secondary events (for example gap junctions) cannot.
* Connections without ``long_range`` use the short path. While morph is active, a short-path spike connection
  must stay within the local group; a connection between neurons on processes of different groups raises an error
  at :py:func:`.Connect`. Secondary connections are exchanged among all processes and need not stay within a
  group.
* ``long_range`` is not available for spatially structured connections, for SONATA networks, and for
  :py:func:`.Connect` with arrays of node IDs.

Placing neurons
---------------

Morph gains only if each area lies within one local group. NEST distributes neurons round-robin over the virtual
processes: the neuron with node ID ``id`` lies on process ``id % N``, where ``N`` is the number of MPI processes.
A strided slice with step ``N`` of the neurons of one call to :py:func:`.Create` therefore lies on one process. To
place one area per process, create all neurons in one call, padded to the size of the largest area times the
number of areas, freeze them, and assign each area a strided slice:

.. code-block:: python

    import nest

    nest.ResetKernel()
    nest.set(threshold_delay=1.0)  # ms; before any connection

    area_sizes = [3000, 2600, 3400, 2800]  # one area per MPI process
    n_areas = len(area_sizes)
    assert nest.NumProcesses() == n_areas

    # all neurons at once, padded to n_areas x largest area; unused neurons stay frozen
    all_neurons = nest.Create("iaf_psc_exp", n_areas * max(area_sizes))
    all_neurons.frozen = True

    # area i is a strided slice with step n_areas, which lies on one process
    areas = []
    for i, size in enumerate(area_sizes):
        area = all_neurons[i : i + size * n_areas : n_areas]
        area.frozen = False
        areas.append(area)

    for i, target in enumerate(areas):
        n_exc = int(0.8 * len(target))
        # within the area: short path
        nest.Connect(target[:n_exc], target, {"rule": "fixed_indegree", "indegree": 100},
                     {"weight": 30.0, "delay": nest.random.uniform(0.1, 1.5)})
        nest.Connect(target[n_exc:], target, {"rule": "fixed_indegree", "indegree": 25},
                     {"weight": -120.0, "delay": nest.random.uniform(0.1, 1.5)})
        # between areas: long path, delays of at least threshold_delay
        for j, source in enumerate(areas):
            if i != j:
                nest.Connect(source[: int(0.8 * len(source))], target,
                             {"rule": "fixed_indegree", "indegree": 50, "long_range": True},
                             {"weight": 30.0, "delay": nest.random.uniform(1.0, 3.0)})

Populations within an area are consecutive pieces of the area's slice; population ``p`` of area ``i`` is
``all_neurons[i + start * n_areas : i + end * n_areas : n_areas]``, where ``start`` and ``end`` count the neurons of
the area before and up to the end of the population.

For local groups of more than one process, give each group a set of consecutive ranks and place each area on the
processes of its group, for example with ``comm_local_ranks`` set to ``[2 * (rank // 2), 2 * (rank // 2) + 1]``
for groups of two processes.

Connections and their status
----------------------------

* :py:func:`.GetConnections` returns the connections of both paths. While morph is active, the connection status
  contains the key ``long_range``.
* ``get`` and ``set`` on a SynapseCollection and :py:func:`.Disconnect` with a SynapseCollection address the path
  on which each connection was created, also if the same pair of neurons is connected on both paths.
* :py:func:`.Disconnect` with two NodeCollections removes connections of the long path if ``conn_spec`` contains
  ``long_range``. Without the key it removes connections of the short path, and connections of the long path if
  the pair has no connection on the short path.

Limitations
-----------

* Structural plasticity creates connections on the short path between any two processes. With morph it is
  therefore available only if one local group contains all processes; otherwise :py:func:`.Simulate` raises an
  error.
* ``threshold_delay`` and ``comm_local_ranks`` cannot change once connections exist.
* Long-range connections cannot be created or removed between :py:func:`.Simulate` calls while spikes of the long
  path are in flight: if the previous call ended within an interval of ``threshold_delay``, or if spikes of the
  long path were exchanged at its end. :py:func:`.Simulate` then raises an error. Connections of the short path
  can change.

References
----------

.. [1] Lober M, Diesmann M, Kunkel S (2026). Exploiting network topology in brain-scale simulations of spiking
       neural networks. Neuromorphic Computing and Engineering 6(2):024024.
       https://doi.org/10.1088/2634-4386/ae762e
